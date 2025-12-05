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
        emit operationCompleted(QString("Loaded %1 logos from %2")
            .arg(m_logos.size())
            .arg(fileInfo.fileName()));
        
        return true;
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Error loading file: %1").arg(e.what()));
        return false;
    }
}

void LogoFile::clearFile() {
    m_logos.clear();
    m_logoImages.clear();
    m_filePath.clear();
    m_projectDir.clear();
    m_headerInfo.clear();
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
    
    if (index < 1 || index > m_logoImages.size()) {
        emit errorOccurred("Invalid logo index");
        return false;
    }
    
    try {
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
        QString filename = QString("logo_%1_%2x%3.png")
            .arg(i + 1)
            .arg(m_logos[i].width)
            .arg(m_logos[i].height);
        
        QString fullPath = dir.filePath(filename);
        if (extractLogo(i + 1, fullPath)) {
            success++;
        }
    }
    
    emit operationCompleted(QString("Extracted %1/%2 logos to %3")
        .arg(success)
        .arg(m_logos.size())
        .arg(outputDir));
    
    return success == m_logos.size();
}

bool LogoFile::replaceLogo(int index, const QString &imagePath) {
    qDebug() << "replaceLogo called: index=" << index << ", path=" << imagePath;
    
    // Check if we're in project mode
    if (m_projectDir.isEmpty()) {
        emit errorOccurred("Replace only works in project mode. Please 'Unpack to Project' first.");
        return false;
    }
    
    if (index < 1 || index > m_logoImages.size()) {
        emit errorOccurred("Invalid logo index");
        return false;
    }
    
    try {
        // Load PNG
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
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Replace failed: %1").arg(e.what()));
        return false;
    }
}

bool LogoFile::saveFile(const QString &outputPath) {
    if (!m_isLoaded) {
        emit errorOccurred("No file loaded");
        return false;
    }
    
    try {
        // Create LogoImage from our blobs
        LogoImage image = LogoImage::createFromBlobs(m_logoImages);
        
        // Write to file
        image.writeToFile(outputPath.toStdString());
        
        emit operationCompleted(QString("Saved to %1").arg(outputPath));
        return true;
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Save failed: %1").arg(e.what()));
        return false;
    }
}

// Native file dialog methods
QString LogoFile::browseForFile() {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString filter = "Logo files (*.bin);;All files (*)";
    
    QString path = QFileDialog::getOpenFileName(
        nullptr,
        "Open Logo File",
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
    QString filter = "Logo files (*.bin);;All files (*)";
    
    QString path = QFileDialog::getSaveFileName(
        nullptr,
        "Save Logo File",
        documentsPath + "/logo.bin",
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
    
    // Extract all logos
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
    
    if (!loadProjectMetadata(projectDir)) {
        return false;
    }
    
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
        // Extract the numeric index from "logo_X_WIDTHxHEIGHT.png"
        QRegularExpression re("logo_(\\d+)_");
        QRegularExpressionMatch matchA = re.match(a);
        QRegularExpressionMatch matchB = re.match(b);
        
        if (matchA.hasMatch() && matchB.hasMatch()) {
            int indexA = matchA.captured(1).toInt();
            int indexB = matchB.captured(1).toInt();
            return indexA < indexB;
        }
        return a < b; // Fallback to alphabetical
    });
    
    clearFile();
    m_projectDir = projectDir;
    m_isLoaded = true;
    
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
    
    emit filePathChanged();
    emit isLoadedChanged();
    emit logoCountChanged();
    emit headerInfoChanged();
    emit isProjectModeChanged();
    emit operationCompleted("Project opened successfully");
    
    return true;
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
    
    QTextStream out(&file);
    out << "{\n";
    out << "  \"version\": \"1.0\",\n";
    out << "  \"type\": \"bootmod-project\",\n";
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
        readme << "This folder contains an unpacked MTK logo.bin file.\n";
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
