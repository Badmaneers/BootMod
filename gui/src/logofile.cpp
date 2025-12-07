#include "logofile.h"
#include "thumbnailprovider.h"
#include "appcontext.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QPixmap>
#include <QStandardPaths>
#include <QDateTime>
#include <QRegularExpression>
#include <QFileDialog>
#include <algorithm>
#include <fstream>

using namespace mtklogo;

LogoFile::LogoFile(QObject *parent) : QObject(parent) {
    // Get thumbnail provider from app context
    m_thumbnailProvider = AppContext::instance()->thumbnailProvider();
}

bool LogoFile::loadFile(const QString &path) {
    clearFile();
    
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        emit errorOccurred("File does not exist: " + path);
        return false;
    }
    
    // Detect file format
    m_currentFormat = bootmod::detectFormat(path.toStdString());
    
    if (m_currentFormat == bootmod::FormatType::MTK_LOGO) {
        m_formatType = "MediaTek";
        return loadMtkFile(path);
    } else if (m_currentFormat == bootmod::FormatType::OPPO_SPLASH) {
        m_formatType = "Snapdragon";
        return loadSplashFile(path);
    } else {
        emit errorOccurred(QString("Unsupported file format: %1\n\n"
                                   "This tool supports:\n"
                                   "• MediaTek logo.bin files\n"
                                   "• Qualcomm/Snapdragon splash.img files\n\n"
                                   "The selected file does not match either format.")
                          .arg(fileInfo.fileName()));
        return false;
    }
}

bool LogoFile::loadMtkFile(const QString &path) {
    QFileInfo fileInfo(path);
    
    try {
        // Load the logo image
        LogoImage logoImage = LogoImage::readFromFile(path.toStdString());
        
        if (logoImage.getLogoCount() == 0) {
            emit errorOccurred("No logos found in file");
            return false;
        }
        
        m_logoImages.clear();
        for (size_t i = 0; i < logoImage.blobs.size(); ++i) {
            m_logoImages.push_back(logoImage.blobs[i]);
        }
        
        // Extract header info
        m_headerInfo = QString("Logos: %1 | Block Size: %2")
            .arg(logoImage.table.logo_count)
            .arg(logoImage.table.block_size);
        
        // Create entries with placeholder data
        for (size_t i = 0; i < m_logoImages.size(); ++i) {
            LogoEntry entry;
            entry.index = i + 1;
            entry.size = m_logoImages[i].size();
            entry.format = "Compressed";
            
            // Decompress to get dimensions
            try {
                auto decompressed = ImageUtils::zlibDecompress(m_logoImages[i]);
                
                // Guess dimensions - try BGRA first
                auto dims = MtkLogo::guessDimensions(decompressed.size(), ColorMode::BGRA_LE);
                if (!dims.empty()) {
                    entry.width = dims[0].first;
                    entry.height = dims[0].second;
                    entry.format = "BGRA";
                } else {
                    // Try RGB565
                    dims = MtkLogo::guessDimensions(decompressed.size(), ColorMode::RGB565_LE);
                    if (!dims.empty()) {
                        entry.width = dims[0].first;
                        entry.height = dims[0].second;
                        entry.format = "RGB565";
                    } else {
                        entry.width = 0;
                        entry.height = 0;
                    }
                }
                
                // Create QImage for thumbnail
                if (entry.width > 0 && entry.height > 0) {
                    QImage image;
                    if (entry.format == "BGRA") {
                        image = QImage(entry.width, entry.height, QImage::Format_RGBA8888);
                        const uint8_t* src = decompressed.data();
                        
                        for (int y = 0; y < entry.height; ++y) {
                            uint8_t* dst = image.scanLine(y);
                            for (int x = 0; x < entry.width; ++x) {
                                // BGRA to RGBA
                                dst[x*4 + 0] = src[x*4 + 2]; // R
                                dst[x*4 + 1] = src[x*4 + 1]; // G
                                dst[x*4 + 2] = src[x*4 + 0]; // B
                                dst[x*4 + 3] = src[x*4 + 3]; // A
                            }
                            src += entry.width * 4;
                        }
                    } else if (entry.format == "RGB565") {
                        image = QImage(entry.width, entry.height, QImage::Format_RGB888);
                        const uint16_t* src = reinterpret_cast<const uint16_t*>(decompressed.data());
                        
                        for (int y = 0; y < entry.height; ++y) {
                            uint8_t* dst = image.scanLine(y);
                            for (int x = 0; x < entry.width; ++x) {
                                uint16_t pixel = src[y * entry.width + x];
                                dst[x*3 + 0] = ((pixel >> 11) & 0x1F) << 3; // R
                                dst[x*3 + 1] = ((pixel >> 5) & 0x3F) << 2;  // G
                                dst[x*3 + 2] = (pixel & 0x1F) << 3;          // B
                            }
                        }
                    }
                    
                    entry.thumbnail = createThumbnail(image);
                    
                    // Add to thumbnail provider if available
                    if (m_thumbnailProvider) {
                        m_thumbnailProvider->addThumbnail(entry.index, QPixmap::fromImage(entry.thumbnail));
                    }
                }
            } catch (...) {
                entry.width = 0;
                entry.height = 0;
            }
            
            m_logos.append(entry);
        }
        
        m_filePath = path;
        m_isLoaded = true;
        
        emit filePathChanged();
        emit isLoadedChanged();
        emit logoCountChanged();
        emit headerInfoChanged();
        emit formatTypeChanged();
        emit operationCompleted(QString("Loaded %1 logos from %2")
            .arg(m_logos.size())
            .arg(fileInfo.fileName()));
        
        return true;
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Error loading file: %1").arg(e.what()));
        return false;
    }
}

bool LogoFile::loadSplashFile(const QString &path) {
    QFileInfo fileInfo(path);
    
    try {
        // Create SplashImage instance
        m_splashImage = std::make_unique<bootmod::splash::SplashImage>();
        
        if (!m_splashImage->load(path.toStdString())) {
            emit errorOccurred("Failed to load splash.img");
            return false;
        }
        
        uint32_t imageCount = m_splashImage->getImageCount();
        if (imageCount == 0) {
            emit errorOccurred("No images found in splash.img");
            return false;
        }
        
        // Extract header info
        m_headerInfo = QString("Images: %1 | Resolution: %2x%3 | Format: OPPO/OnePlus")
            .arg(imageCount)
            .arg(m_splashImage->getWidth())
            .arg(m_splashImage->getHeight());
        
        // Create entries for each image
        for (uint32_t i = 0; i < imageCount; ++i) {
            auto info = m_splashImage->getImageInfo(i);
            
            LogoEntry entry;
            entry.index = i + 1;
            entry.width = info.width;
            entry.height = info.height;
            entry.size = info.compressed_size;
            entry.format = "BMP+gzip";
            
            // Get decompressed image data for thumbnail
            uint32_t width, height;
            auto bmpData = m_splashImage->getImageData(i, width, height);
            
            if (!bmpData.empty() && bmpData.size() >= 54) {
                // Parse BMP to create thumbnail
                uint16_t bpp = *reinterpret_cast<uint16_t*>(&bmpData[28]);
                uint32_t offset = *reinterpret_cast<uint32_t*>(&bmpData[10]);
                
                QImage image(width, height, QImage::Format_RGBA8888);
                const uint8_t* bmp_pixels = bmpData.data() + offset;
                int bytes_per_pixel = bpp / 8;
                int row_size = ((width * bytes_per_pixel + 3) / 4) * 4;
                
                for (uint32_t y = 0; y < height; ++y) {
                    const uint8_t* src_row = bmp_pixels + (height - 1 - y) * row_size;
                    uint8_t* dst_row = image.scanLine(y);
                    
                    for (uint32_t x = 0; x < width; ++x) {
                        if (bpp == 24) {
                            dst_row[x*4 + 0] = src_row[x*3 + 2]; // R = B
                            dst_row[x*4 + 1] = src_row[x*3 + 1]; // G = G
                            dst_row[x*4 + 2] = src_row[x*3 + 0]; // B = R
                            dst_row[x*4 + 3] = 255;              // A
                        } else if (bpp == 32) {
                            dst_row[x*4 + 0] = src_row[x*4 + 2]; // R = B
                            dst_row[x*4 + 1] = src_row[x*4 + 1]; // G = G
                            dst_row[x*4 + 2] = src_row[x*4 + 0]; // B = R
                            dst_row[x*4 + 3] = src_row[x*4 + 3]; // A = A
                        }
                    }
                }
                
                entry.thumbnail = createThumbnail(image);
                
                // Add to thumbnail provider
                if (m_thumbnailProvider) {
                    m_thumbnailProvider->addThumbnail(entry.index, QPixmap::fromImage(entry.thumbnail));
                }
            }
            
            m_logos.append(entry);
        }
        
        m_filePath = path;
        m_isLoaded = true;
        
        emit filePathChanged();
        emit isLoadedChanged();
        emit logoCountChanged();
        emit headerInfoChanged();
        emit formatTypeChanged();
        emit operationCompleted(QString("Loaded %1 images from %2")
            .arg(m_logos.size())
            .arg(fileInfo.fileName()));
        
        return true;
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Error loading splash.img: %1").arg(e.what()));
        return false;
    }
}

void LogoFile::clearFile() {
    m_logos.clear();
    m_logoImages.clear();
    m_splashImage.reset();
    m_filePath.clear();
    m_projectDir.clear();
    m_headerInfo.clear();
    m_formatType.clear();
    m_isLoaded = false;
    
    if (m_thumbnailProvider) {
        m_thumbnailProvider->clear();
    }
    
    emit filePathChanged();
    emit isLoadedChanged();
    emit logoCountChanged();
    emit headerInfoChanged();
    emit isProjectModeChanged();
}

bool LogoFile::extractLogo(int index, const QString &outputPath) {
    qDebug() << "extractLogo called: index=" << index << ", path=" << outputPath;
    
    if (index < 1 || index > m_logos.size()) {
        emit errorOccurred("Invalid logo index");
        return false;
    }
    
    try {
        if (m_currentFormat == bootmod::FormatType::OPPO_SPLASH) {
            // Snapdragon splash.img extraction
            if (!m_splashImage) {
                emit errorOccurred("Splash image not loaded");
                return false;
            }
            
            if (!m_splashImage->extractImage(index - 1, outputPath.toStdString())) {
                emit errorOccurred("Failed to extract image");
                return false;
            }
            
            emit operationCompleted(QString("Exported image #%1 to %2").arg(index).arg(outputPath));
            return true;
            
        } else {
            // MediaTek logo.bin extraction
            if (index > m_logoImages.size()) {
                emit errorOccurred("Invalid logo index");
                return false;
            }
            
            // Decompress
            auto decompressed = ImageUtils::zlibDecompress(m_logoImages[index - 1]);
            
            // Get dimensions from our stored entry
            const LogoEntry& entry = m_logos[index - 1];
            
            // Determine color mode
            ColorMode mode = (entry.format == "RGB565") ? ColorMode::RGB565_LE : ColorMode::BGRA_LE;
            
            // Save as PNG
            bool success = ImageUtils::saveToPNG(outputPath.toStdString(), decompressed, 
                                                 entry.width, entry.height, mode);
            
            if (success) {
                emit operationCompleted(QString("Exported logo #%1 to %2").arg(index).arg(outputPath));
            } else {
                emit errorOccurred("Failed to save PNG");
            }
            
            return success;
        }
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Export failed: %1").arg(e.what()));
        return false;
    }
}

bool LogoFile::extractAll(const QString &outputDir) {
    QDir dir(outputDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    int success = 0;
    for (int i = 0; i < m_logos.size(); ++i) {
        QString filename;
        if (m_currentFormat == bootmod::FormatType::OPPO_SPLASH) {
            filename = QString("image_%1.png").arg(i);
        } else {
            filename = QString("logo_%1_%2x%3.png")
                .arg(i + 1)
                .arg(m_logos[i].width)
                .arg(m_logos[i].height);
        }
        
        QString fullPath = dir.filePath(filename);
        if (extractLogo(i + 1, fullPath)) {
            success++;
        }
    }
    
    emit operationCompleted(QString("Extracted %1/%2 %3 to %4")
        .arg(success)
        .arg(m_logos.size())
        .arg(m_currentFormat == bootmod::FormatType::OPPO_SPLASH ? "images" : "logos")
        .arg(outputDir));
    
    return success == m_logos.size();
}

bool LogoFile::replaceLogo(int index, const QString &imagePath) {
    qDebug() << "replaceLogo called: index=" << index << ", path=" << imagePath;
    qDebug() << "  m_projectDir=" << m_projectDir;
    qDebug() << "  m_currentFormat=" << static_cast<int>(m_currentFormat);
    qDebug() << "  m_logoImages.size()=" << m_logoImages.size();
    qDebug() << "  m_splashImage=" << (m_splashImage ? "valid" : "null");
    
    // Check if we're in project mode
    if (m_projectDir.isEmpty()) {
        emit errorOccurred("Replace only works in project mode. Please 'Unpack to Project' first.");
        return false;
    }
    
    if (index < 1 || index > m_logos.size()) {
        emit errorOccurred(QString("Invalid logo index: %1 (valid: 1-%2)").arg(index).arg(m_logos.size()));
        return false;
    }
    
    try {
        if (m_currentFormat == bootmod::FormatType::OPPO_SPLASH) {
            // Handle Snapdragon splash.img format
            qDebug() << "Replacing Snapdragon splash image...";
            
            if (!m_splashImage) {
                emit errorOccurred("No splash image loaded");
                return false;
            }
            
            qDebug() << "Calling m_splashImage->replaceImage(" << (index - 1) << ")...";
            
            // Replace image in splash (index is 1-based in GUI, 0-based in API)
            if (!m_splashImage->replaceImage(index - 1, imagePath.toStdString())) {
                emit errorOccurred("Failed to replace splash image");
                return false;
            }
            
            qDebug() << "Replace successful, updating project file...";
            
            // Update the project file
            QString imagesDir = QDir(m_projectDir).filePath("images");
            QDir dir(imagesDir);
            QString pattern = QString("image_%1.png").arg(index - 1);
            
            // Copy the new image to project directory
            QString outputPath = QDir(imagesDir).filePath(pattern);
            if (QFile::exists(outputPath)) {
                qDebug() << "Removing existing file:" << outputPath;
                QFile::remove(outputPath);
            }
            
            qDebug() << "Copying to:" << outputPath;
            if (!QFile::copy(imagePath, outputPath)) {
                emit errorOccurred("Failed to update project file");
                return false;
            }
            
            // Update thumbnail
            QImage newImage(outputPath);
            if (!newImage.isNull()) {
                m_logos[index - 1].thumbnail = newImage.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                
                // Update thumbnail provider
                if (m_thumbnailProvider) {
                    m_thumbnailProvider->addThumbnail(index, QPixmap::fromImage(m_logos[index - 1].thumbnail));
                }
                
                emit logoCountChanged(); // Trigger refresh
            }
            
            qDebug() << "Replace complete!";
            emit operationCompleted(QString("Replaced splash image %1").arg(index));
            return true;
            
        } else if (m_currentFormat == bootmod::FormatType::MTK_LOGO) {
            // Handle MediaTek logo.bin format
            qDebug() << "Replacing MTK logo...";
            uint32_t width, height;
            const LogoEntry& entry = m_logos[index - 1];
            ColorMode mode = (entry.format == "RGB565") ? ColorMode::RGB565_LE : ColorMode::BGRA_LE;
            
            auto pixels = ImageUtils::loadFromPNG(imagePath.toStdString(), width, height, mode);
            
            // Validate dimensions
            if (width != entry.width || height != entry.height) {
                emit errorOccurred(QString("Dimension mismatch: Expected %1x%2, got %3x%4")
                    .arg(entry.width).arg(entry.height)
                    .arg(width).arg(height));
                return false;
            }
            
            // Compress
            auto compressed = ImageUtils::zlibCompress(pixels, 9);
            
            // Replace in memory
            m_logoImages[index - 1] = compressed;
            
            // Find the existing logo file in the project
            QString imagesDir = QDir(m_projectDir).filePath("images");
            QDir dir(imagesDir);
            QString pattern = QString("logo_%1_*.png").arg(index);
            QStringList existingFiles = dir.entryList(QStringList() << pattern, QDir::Files);
            
            // Delete old file if it exists
            if (!existingFiles.isEmpty()) {
                QString oldFile = QDir(imagesDir).filePath(existingFiles.first());
                qDebug() << "Deleting old logo file:" << oldFile;
                QFile::remove(oldFile);
            }
            
            // Save to project directory with dimensions in filename
            QString filename = QString("logo_%1_%2x%3.png")
                .arg(index)
                .arg(width)
                .arg(height);
            QString outputPath = QDir(imagesDir).filePath(filename);
            
                qDebug() << "Saving replaced logo to:" << outputPath;
            
            // Save the new PNG to project folder
            bool success = ImageUtils::saveToPNG(outputPath.toStdString(), pixels, 
                                                 width, height, mode);
            
            if (!success) {
                emit errorOccurred("Failed to save PNG to project folder");
                return false;
            }
            
            // Update the entry and thumbnail in memory (instead of reloading entire project)
            m_logoImages[index - 1] = compressed;
            
            // Load the new image and update thumbnail
            QImage newImage(outputPath);
            if (!newImage.isNull()) {
                m_logos[index - 1].thumbnail = createThumbnail(newImage);
                m_logos[index - 1].rawData = compressed;
                m_logos[index - 1].size = compressed.size();
                
                // Update thumbnail provider
                if (m_thumbnailProvider) {
                    m_thumbnailProvider->addThumbnail(index, QPixmap::fromImage(m_logos[index - 1].thumbnail));
                }
                
                // Force QML to refresh by emitting data changed signal
                emit logoCountChanged(); // This triggers GridView to refresh
            }
            
            emit operationCompleted(QString("Replaced logo #%1 in project").arg(index));
            return true;
            
        } else {
            emit errorOccurred("Unknown file format");
            return false;
        }
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Replace failed: %1").arg(e.what()));
        return false;
    }
    
    // Should never reach here
    return false;
}

bool LogoFile::saveFile(const QString &outputPath) {
    qDebug() << "saveFile called: outputPath=" << outputPath;
    qDebug() << "  m_isLoaded=" << m_isLoaded;
    qDebug() << "  m_currentFormat=" << static_cast<int>(m_currentFormat);
    qDebug() << "  m_splashImage=" << (m_splashImage ? "valid" : "null");
    
    if (!m_isLoaded) {
        emit errorOccurred("No file loaded");
        return false;
    }
    
    try {
        if (m_currentFormat == bootmod::FormatType::OPPO_SPLASH) {
            // Save Snapdragon splash.img
            qDebug() << "Saving Snapdragon splash.img...";
            
            if (!m_splashImage) {
                emit errorOccurred("No splash image data loaded");
                return false;
            }
            
            qDebug() << "Calling m_splashImage->save()...";
            if (!m_splashImage->save(outputPath.toStdString())) {
                emit errorOccurred("Failed to save splash.img");
                return false;
            }
            
            qDebug() << "Save successful!";
            emit operationCompleted(QString("Saved splash.img to %1").arg(outputPath));
            return true;
            
        } else if (m_currentFormat == bootmod::FormatType::MTK_LOGO) {
            // Save MediaTek logo.bin
            LogoImage image = LogoImage::createFromBlobs(m_logoImages);
            image.writeToFile(outputPath.toStdString());
            
            emit operationCompleted(QString("Saved logo.bin to %1").arg(outputPath));
            return true;
            
        } else {
            emit errorOccurred("Unknown file format");
            return false;
        }
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Save failed: %1").arg(e.what()));
        return false;
    }
}

// Native file dialog methods
QString LogoFile::browseForFile() {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString filter = "Boot Logo files (*.bin *.img);;Logo.bin (*.bin);;Splash.img (*.img);;All files (*)";
    
    QString path = QFileDialog::getOpenFileName(
        nullptr,
        "Open Boot Logo/Splash File",
        documentsPath,
        filter
    );
    
    if (!path.isEmpty()) {
        loadFile(path);
    }
    
    return path;
}

QString LogoFile::browseForSaveFile() {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    
    // Use appropriate default filename based on current format
    QString defaultFilename;
    if (m_currentFormat == bootmod::FormatType::OPPO_SPLASH) {
        defaultFilename = "/splash.img";
    } else {
        defaultFilename = "/logo.bin";
    }
    
    QString filter = "Boot Logo files (*.bin *.img);;Logo.bin (*.bin);;Splash.img (*.img);;All files (*)";
    
    QString path = QFileDialog::getSaveFileName(
        nullptr,
        "Save Boot Logo/Splash File",
        documentsPath + defaultFilename,
        filter
    );
    
    if (!path.isEmpty()) {
        saveFile(path);
    }
    
    return path;
}

QString LogoFile::browseForFolder() {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    
    QString path = QFileDialog::getExistingDirectory(
        nullptr,
        "Select Directory",
        documentsPath,
        QFileDialog::ShowDirsOnly
    );
    
    return path;
}

QString LogoFile::browseForImage() {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString filter = "PNG images (*.png);;All files (*)";
    
    QString path = QFileDialog::getOpenFileName(
        nullptr,
        "Select PNG Image",
        documentsPath,
        filter
    );
    
    return path;
}

void LogoFile::browseAndExtractLogo(int index) {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString defaultFilename = QString("/logo_%1.png").arg(index);
    
    QString path = QFileDialog::getSaveFileName(
        nullptr,
        "Export Logo as PNG",
        documentsPath + defaultFilename,
        "PNG images (*.png);;All files (*)"
    );
    
    if (!path.isEmpty()) {
        extractLogo(index, path);
    }
}

void LogoFile::browseAndReplaceLogo(int index) {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    
    QString path = QFileDialog::getOpenFileName(
        nullptr,
        "Select PNG Image to Replace Logo",
        documentsPath,
        "PNG images (*.png);;All files (*)"
    );
    
    if (!path.isEmpty()) {
        replaceLogo(index, path);
    }
}

QImage LogoFile::createThumbnail(const QImage &source, int maxSize) {
    if (source.isNull()) {
        return QImage();
    }
    
    if (source.width() <= maxSize && source.height() <= maxSize) {
        return source;
    }
    
    return source.scaled(maxSize, maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

bool LogoFile::unpackToProject(const QString &logoPath, const QString &projectDir) {
    QDir dir(projectDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            emit errorOccurred("Failed to create project directory");
            return false;
        }
    }
    
    // Check if directory is empty (except hidden files)
    QStringList existingFiles = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    if (!existingFiles.isEmpty()) {
        emit errorOccurred("Project directory must be empty");
        return false;
    }
    
    // Load the logo file
    if (!loadFile(logoPath)) {
        return false;
    }
    
    // Create images subdirectory
    QString imagesDir = projectDir + "/images";
    if (!QDir().mkpath(imagesDir)) {
        emit errorOccurred("Failed to create images directory");
        return false;
    }
    
    // Extract all logos/images based on format
    if (m_currentFormat == bootmod::FormatType::OPPO_SPLASH) {
        // Snapdragon format: use image_N.png naming
        for (const auto& logo : m_logos) {
            QString filename = QString("image_%1.png").arg(logo.index - 1); // 0-based for splash
            QString outputPath = imagesDir + "/" + filename;
            
            if (!extractLogo(logo.index, outputPath)) {
                emit errorOccurred(QString("Failed to extract image #%1").arg(logo.index));
                return false;
            }
        }
    } else {
        // MediaTek format: use logo_N_WxH.png naming
        for (const auto& logo : m_logos) {
            QString filename = QString("logo_%1_%2x%3.png")
                .arg(logo.index)
                .arg(logo.width)
                .arg(logo.height);
            QString outputPath = imagesDir + "/" + filename;
            
            if (!extractLogo(logo.index, outputPath)) {
                emit errorOccurred(QString("Failed to extract logo #%1").arg(logo.index));
                return false;
            }
        }
    }
    
    // Create project identifier and metadata
    if (!createProjectIdentifier(projectDir)) {
        return false;
    }
    
    m_projectDir = projectDir;
    m_filePath = logoPath;
    
    emit isProjectModeChanged();
    emit operationCompleted("Project created successfully");
    return true;
}

bool LogoFile::openProject(const QString &projectDir) {
    if (!isProjectFolder(projectDir)) {
        emit errorOccurred("Not a valid BootMod project folder");
        return false;
    }
    
    // Read project metadata to determine format
    QString identifierPath = projectDir + "/.bootmod";
    QFile file(identifierPath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred("Failed to read project metadata");
        return false;
    }
    
    QString content = file.readAll();
    file.close();
    
    // Extract format type
    QRegularExpression formatRegex("\"format\":\\s*\"(\\w+)\"");
    auto formatMatch = formatRegex.match(content);
    QString formatStr = formatMatch.hasMatch() ? formatMatch.captured(1) : "mtk";
    
    clearFile();
    m_projectDir = projectDir;
    m_isLoaded = true;
    
    // Determine format
    if (formatStr == "snapdragon") {
        m_currentFormat = bootmod::FormatType::OPPO_SPLASH;
        m_formatType = "Snapdragon";
        
        // For Snapdragon, we need to reload the original splash.img to get m_splashImage
        // Check if original file path is stored
        QRegularExpression origFileRegex("\"original_file\":\\s*\"([^\"]+)\"");
        auto origFileMatch = origFileRegex.match(content);
        
        if (origFileMatch.hasMatch()) {
            QString origFile = origFileMatch.captured(1);
            // Try to find the original file - first try as absolute path, then relative to project dir
            QString searchPath = origFile;
            
            if (!QFile::exists(searchPath)) {
                // Try relative to project directory
                QFileInfo projectInfo(projectDir);
                searchPath = projectInfo.dir().filePath(origFile);
            }
            
            if (!QFile::exists(searchPath)) {
                // Try just the filename in the project directory
                QFileInfo origInfo(origFile);
                searchPath = QFileInfo(projectDir).dir().filePath(origInfo.fileName());
            }
            
            if (QFile::exists(searchPath)) {
                qDebug() << "Reloading original splash.img from:" << searchPath;
                // Load the original splash to get the SplashImage object
                if (loadSplashFile(searchPath)) {
                    qDebug() << "Successfully reloaded splash.img, m_splashImage is valid";
                    // Now replace images from project folder
                    QString imagesDir = projectDir + "/images";
                    QDir dir(imagesDir);
                    QStringList imageFiles = dir.entryList(QStringList() << "image_*.png", QDir::Files, QDir::Name);
                    
                    for (const QString& filename : imageFiles) {
                        // Extract index from image_N.png
                        QRegularExpression re("image_(\\d+)\\.png");
                        auto match = re.match(filename);
                        if (match.hasMatch()) {
                            int index = match.captured(1).toInt();
                            QString imagePath = dir.filePath(filename);
                            
                            qDebug() << "Replacing image" << index << "from" << imagePath;
                            // Replace in m_splashImage
                            if (!m_splashImage->replaceImage(index, imagePath.toStdString())) {
                                qWarning() << "Failed to replace image" << index;
                            }
                        }
                    }
                    
                    // Now update the m_logos list with the modified splash data
                    // This reuses the code from loadSplashFile
                    m_logos.clear();
                    if (m_thumbnailProvider) {
                        m_thumbnailProvider->clear();
                    }
                    
                    uint32_t imageCount = m_splashImage->getImageCount();
                    for (uint32_t i = 0; i < imageCount; i++) {
                        auto info = m_splashImage->getImageInfo(i);
                        uint32_t width = info.width;
                        uint32_t height = info.height;
                        
                        // Get image data as BMP
                        auto bmpData = m_splashImage->getImageData(i, width, height);
                        
                        // Convert BMP to QImage for thumbnail
                        QImage image;
                        if (!image.loadFromData(bmpData.data(), bmpData.size(), "BMP")) {
                            qWarning() << "Failed to load BMP for image" << i;
                            continue;
                        }
                        
                        LogoEntry entry;
                        entry.index = i;
                        entry.width = width;
                        entry.height = height;
                        entry.format = "BMP";
                        entry.size = bmpData.size();
                        entry.thumbnail = createThumbnail(image);
                        entry.rawData = bmpData;
                        
                        m_logos.append(entry);
                        
                        if (m_thumbnailProvider) {
                            m_thumbnailProvider->addThumbnail(i, QPixmap::fromImage(entry.thumbnail));
                        }
                    }
                    
                    qDebug() << "Loaded" << imageCount << "images from Snapdragon project";
                    
                } else {
                    emit errorOccurred(QString("Could not reload original splash.img from: %1\n\nPlace the original splash.img file in the same folder as the project.").arg(searchPath));
                    return false;
                }
            } else {
                emit errorOccurred(QString("Original file not found: %1\n\nSearched locations:\n• %1 (absolute)\n• %2 (relative to project)\n• %3 (in project parent folder)\n\nPlace the original splash.img in one of these locations.").arg(origFile, origFile, QFileInfo(projectDir).dir().filePath(QFileInfo(origFile).fileName())));
                return false;
            }
        } else {
            // No original file specified in metadata
            emit errorOccurred("Project metadata does not contain original file path.\n\nThis Snapdragon project requires the original splash.img file to be opened.");
            return false;
        }
        
    } else {
        // MediaTek format
        m_currentFormat = bootmod::FormatType::MTK_LOGO;
        m_formatType = "MediaTek";
        
        // Load all images from the images directory
        QString imagesDir = projectDir + "/images";
        QDir dir(imagesDir);
        QStringList imageFiles = dir.entryList(QStringList() << "logo_*.png", QDir::Files, QDir::Name);
        
        if (imageFiles.isEmpty()) {
            emit errorOccurred("No logo images found in project");
            return false;
        }
        
        // Sort files by numeric index (logo_1_*.png, logo_2_*.png, etc.)
        std::sort(imageFiles.begin(), imageFiles.end(), [](const QString& a, const QString& b) {
            QRegularExpression re("logo_(\\d+)_");
            QRegularExpressionMatch matchA = re.match(a);
            QRegularExpressionMatch matchB = re.match(b);
            
            if (matchA.hasMatch() && matchB.hasMatch()) {
                int indexA = matchA.captured(1).toInt();
                int indexB = matchB.captured(1).toInt();
                return indexA < indexB;
            }
            return a < b;
        });
        
        // Load each image and create blobs
        for (const QString& filename : imageFiles) {
            QString imagePath = imagesDir + "/" + filename;
            QImage image(imagePath);
            
            if (image.isNull()) {
                emit errorOccurred(QString("Failed to load: %1").arg(filename));
                continue;
            }
            
            // Load PNG and convert to raw BGRA, then compress
            try {
                uint32_t width = 0, height = 0;
                auto rawPixels = ImageUtils::loadFromPNG(imagePath.toStdString(), width, height, ColorMode::BGRA_LE);
                auto compressedBlob = ImageUtils::zlibCompress(rawPixels);
                
                m_logoImages.push_back(compressedBlob);
                
                // Create entry
                LogoEntry entry;
                entry.index = m_logos.size() + 1;
                entry.width = width;
                entry.height = height;
                entry.format = "BGRA8888";
                entry.size = compressedBlob.size();
                entry.thumbnail = createThumbnail(image);
                entry.rawData = compressedBlob;
                
                m_logos.append(entry);
                
                // Add to thumbnail provider
                if (m_thumbnailProvider) {
                    m_thumbnailProvider->addThumbnail(entry.index, QPixmap::fromImage(entry.thumbnail));
                }
                
            } catch (const std::exception& e) {
                emit errorOccurred(QString("Failed to process %1: %2").arg(filename).arg(e.what()));
            }
        }
    }
    
    emit filePathChanged();
    emit isLoadedChanged();
    emit logoCountChanged();
    emit headerInfoChanged();
    emit isProjectModeChanged();
    emit formatTypeChanged();
    emit operationCompleted("Project opened successfully");
    
    return true;
}

void LogoFile::rescanProjectImages() {
    if (m_projectDir.isEmpty() || !m_isLoaded) {
        qWarning() << "Cannot rescan: no project loaded";
        return;
    }
    
    qDebug() << "Rescanning project images from:" << m_projectDir;
    
    // For MediaTek projects, rescan the images folder
    if (m_currentFormat == bootmod::FormatType::MTK_LOGO) {
        QString imagesDir = m_projectDir + "/images";
        QDir dir(imagesDir);
        
        // Force refresh the directory cache
        dir.refresh();
        
        QStringList imageFiles = dir.entryList(QStringList() << "logo_*.png", QDir::Files, QDir::Name);
        
        qDebug() << "Found" << imageFiles.size() << "image files";
        if (!imageFiles.isEmpty()) {
            qDebug() << "  First file:" << imageFiles.first();
            qDebug() << "  Last file:" << imageFiles.last();
        }
        
        // Clear current logos
        m_logos.clear();
        m_logoImages.clear();
        
        // Sort files by numeric index
        std::sort(imageFiles.begin(), imageFiles.end(), [](const QString& a, const QString& b) {
            QRegularExpression re("logo_(\\d+)_");
            QRegularExpressionMatch matchA = re.match(a);
            QRegularExpressionMatch matchB = re.match(b);
            
            if (matchA.hasMatch() && matchB.hasMatch()) {
                int indexA = matchA.captured(1).toInt();
                int indexB = matchB.captured(1).toInt();
                return indexA < indexB;
            }
            return a < b;
        });
        
        // Load each image
        for (const QString& filename : imageFiles) {
            QString imagePath = imagesDir + "/" + filename;
            QImage image(imagePath);
            
            if (image.isNull()) {
                qWarning() << "Failed to load:" << filename;
                continue;
            }
            
            try {
                uint32_t width = 0, height = 0;
                auto rawPixels = ImageUtils::loadFromPNG(imagePath.toStdString(), width, height, ColorMode::BGRA_LE);
                auto compressedBlob = ImageUtils::zlibCompress(rawPixels);
                
                m_logoImages.push_back(compressedBlob);
                
                LogoEntry entry;
                entry.index = m_logos.size() + 1;
                entry.width = width;
                entry.height = height;
                entry.format = "BGRA8888";
                entry.size = compressedBlob.size();
                entry.thumbnail = createThumbnail(image);
                entry.rawData = compressedBlob;
                
                m_logos.append(entry);
                
                if (m_thumbnailProvider) {
                    m_thumbnailProvider->addThumbnail(entry.index, QPixmap::fromImage(entry.thumbnail));
                }
                
            } catch (const std::exception& e) {
                qWarning() << "Failed to process" << filename << ":" << e.what();
            }
        }
        
        qDebug() << "Rescan complete. Logo count:" << m_logos.size();
        emit logoCountChanged();
    }
}

bool LogoFile::saveProject() {
    if (m_projectDir.isEmpty()) {
        emit errorOccurred("No project loaded");
        return false;
    }
    
    // Save all modified logos back to PNG files
    QString imagesDir = m_projectDir + "/images";
    
    for (int i = 0; i < m_logos.size(); ++i) {
        const auto& logo = m_logos[i];
        QString filename = QString("logo_%1_%2x%3.png")
            .arg(logo.index)
            .arg(logo.width)
            .arg(logo.height);
        QString outputPath = imagesDir + "/" + filename;
        
        if (!extractLogo(logo.index, outputPath)) {
            emit errorOccurred(QString("Failed to save logo #%1").arg(logo.index));
            return false;
        }
    }
    
    emit operationCompleted("Project saved successfully");
    return true;
}

bool LogoFile::exportProject(const QString &outputPath) {
    if (!m_isLoaded || m_logoImages.empty()) {
        emit errorOccurred("No project loaded");
        return false;
    }
    
    return saveFile(outputPath);
}

bool LogoFile::isProjectFolder(const QString &path) {
    QDir dir(path);
    return dir.exists() && 
           QFile::exists(path + "/.bootmod") &&
           QDir(path + "/images").exists();
}

bool LogoFile::createProjectIdentifier(const QString &projectDir) {
    QString identifierPath = projectDir + "/.bootmod";
    QFile file(identifierPath);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Failed to create project identifier");
        return false;
    }
    
    // Determine format name
    QString formatName = "unknown";
    if (m_currentFormat == bootmod::FormatType::MTK_LOGO) {
        formatName = "mtk";
    } else if (m_currentFormat == bootmod::FormatType::OPPO_SPLASH) {
        formatName = "snapdragon";
    }
    
    QTextStream out(&file);
    out << "{\n";
    out << "  \"version\": \"1.0\",\n";
    out << "  \"type\": \"bootmod-project\",\n";
    out << "  \"format\": \"" << formatName << "\",\n";
    out << "  \"created\": \"" << QDateTime::currentDateTime().toString(Qt::ISODate) << "\",\n";
    out << "  \"tool\": \"BootMod GUI\",\n";
    out << "  \"logo_count\": " << m_logos.size() << ",\n";
    out << "  \"original_file\": \"" << QFileInfo(m_filePath).fileName() << "\"\n";
    out << "}\n";
    
    file.close();
    
    // Also create a README
    QString readmePath = projectDir + "/README.txt";
    QFile readmeFile(readmePath);
    if (readmeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream readme(&readmeFile);
        readme << "BootMod Project\n";
        readme << "===============\n\n";
        
        if (m_currentFormat == bootmod::FormatType::OPPO_SPLASH) {
            readme << "This folder contains an unpacked Snapdragon splash.img file.\n";
        } else {
            readme << "This folder contains an unpacked MediaTek logo.bin file.\n";
        }
        
        readme << "You can edit the PNG images in the 'images' folder.\n\n";
        readme << "To repack, open this folder in BootMod GUI and use Export.\n\n";
        readme << "DO NOT DELETE the .bootmod file - it identifies this as a project folder.\n";
        readmeFile.close();
    }
    
    return true;
}

bool LogoFile::loadProjectMetadata(const QString &projectDir) {
    QString identifierPath = projectDir + "/.bootmod";
    QFile file(identifierPath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred("Failed to read project metadata");
        return false;
    }
    
    // Simple JSON-like parsing (could use QJsonDocument for robustness)
    QString content = file.readAll();
    file.close();
    
    // Extract logo count for info
    QRegularExpression logoCountRegex("\"logo_count\":\\s*(\\d+)");
    auto match = logoCountRegex.match(content);
    if (match.hasMatch()) {
        int logoCount = match.captured(1).toInt();
        m_headerInfo = QString("Project | Logos: %1").arg(logoCount);
    }
    
    return true;
}
