#include "launcher.h"
#include "main_window.h"
#include <QApplication>
#include <QMessageBox>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--cli") {
        DoomLauncher launcher;
        launcher.run();
        return 0;
    }

    QApplication application(argc, argv);
    try {
        MainWindow window;
        window.show();
        return application.exec();
    } catch (const std::runtime_error& error) {
        QMessageBox::critical(nullptr, "Doom Launcher", error.what());
        return 1;
    }
}
