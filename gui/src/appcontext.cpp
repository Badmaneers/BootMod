#include "appcontext.h"

AppContext* AppContext::s_instance = nullptr;

AppContext::AppContext(QObject *parent) : QObject(parent) {}

AppContext* AppContext::instance() {
    if (!s_instance) {
        s_instance = new AppContext();
    }
    return s_instance;
}
