#pragma once

#include <QMainWindow>
#include <QProcess>
#include <QStringList>

class QComboBox;
class QLineEdit;
class QListWidget;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void refreshFiles(bool preserveSelection = false);
    void refreshProfiles();
    void loadProfile();
    void saveProfile();
    void deleteProfile();
    void launchGame();
    QString modsDirectory() const;
    QStringList selectedFiles(const QListWidget* list) const;
    void setSelectedFiles(QListWidget* list, const QStringList& paths);

    QString doomDirectory;
    QComboBox* engineCombo = nullptr;
    QComboBox* iwadCombo = nullptr;
    QComboBox* profileCombo = nullptr;
    QLineEdit* profileName = nullptr;
    QLineEdit* sladePath = nullptr;
    QLineEdit* builderPath = nullptr;
    QListWidget* megawadList = nullptr;
    QListWidget* modList = nullptr;
    QProcess gameProcess;
};
