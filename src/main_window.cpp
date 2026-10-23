#include "main_window.h"

#include <QAbstractItemView>
#include <QByteArray>
#include <QComboBox>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSettings>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>
#include <stdexcept>

namespace {

QStringList matchingFiles(const QString& directory, const QStringList& filters, bool recursive = false) {
    QDir dir(directory);
    QStringList paths;
    if (recursive) {
        QDirIterator iterator(
            directory,
            filters,
            QDir::Files | QDir::Readable,
            QDirIterator::Subdirectories
        );
        while (iterator.hasNext()) {
            paths.append(QFileInfo(iterator.next()).absoluteFilePath());
        }
        paths.sort(Qt::CaseInsensitive);
        return paths;
    }

    const QFileInfoList entries = dir.entryInfoList(
        filters,
        QDir::Files | QDir::Readable,
        QDir::Name | QDir::IgnoreCase
    );

    for (const QFileInfo& entry : entries) {
        paths.append(entry.absoluteFilePath());
    }
    return paths;
}

void populateFileList(QListWidget* list, const QStringList& paths) {
    list->clear();
    for (const QString& path : paths) {
        list->addItem(path);
    }
}

QJsonArray toJsonArray(const QStringList& values) {
    QJsonArray array;
    for (const QString& value : values) {
        array.append(value);
    }
    return array;
}

QStringList fromJsonArray(const QJsonValue& value) {
    QStringList values;
    for (const QJsonValue& entry : value.toArray()) {
        if (entry.isString()) {
            values.append(entry.toString());
        }
    }
    return values;
}

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Doom Launcher");
    resize(900, 620);

    doomDirectory = QDir(QDir::homePath()).filePath("Games/DOOM");
    const QStringList directories = {
        "iwads", "megawads", "mods", "profiles",
        "gzd/mods", "zand/mods"
    };
    for (const QString& directory : directories) {
        if (!QDir().mkpath(QDir(doomDirectory).filePath(directory))) {
            throw std::runtime_error(
                ("Could not create " + QDir(doomDirectory).filePath(directory)).toStdString()
            );
        }
    }

    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);
    auto* form = new QFormLayout();

    engineCombo = new QComboBox(central);
    engineCombo->setEditable(true);
    engineCombo->addItems({"gzdoom", "uzdoom", "chocolate-doom", "zandronum"});
    engineCombo->setCurrentText("gzdoom");

    auto* engineRow = new QWidget(central);
    auto* engineLayout = new QHBoxLayout(engineRow);
    engineLayout->setContentsMargins(0, 0, 0, 0);
    auto* browseEngine = new QPushButton("Browse...", engineRow);
    engineLayout->addWidget(engineCombo, 1);
    engineLayout->addWidget(browseEngine);
    form->addRow("Engine", engineRow);

    iwadCombo = new QComboBox(central);
    form->addRow("IWAD", iwadCombo);
    mainLayout->addLayout(form);

    auto* lists = new QHBoxLayout();
    megawadList = new QListWidget(central);
    modList = new QListWidget(central);
    for (QListWidget* list : {megawadList, modList}) {
        list->setSelectionMode(QAbstractItemView::ExtendedSelection);
        list->setDragDropMode(QAbstractItemView::InternalMove);
        list->setDefaultDropAction(Qt::MoveAction);
    }

    auto* megawadGroup = new QVBoxLayout();
    megawadGroup->addWidget(new QLabel("Megawads", central));
    megawadGroup->addWidget(megawadList);
    auto* modGroup = new QVBoxLayout();
    modGroup->addWidget(new QLabel("Mods (drag to set load order)", central));
    modGroup->addWidget(modList);
    lists->addLayout(megawadGroup);
    lists->addLayout(modGroup);
    mainLayout->addLayout(lists, 1);

    auto* profileRow = new QHBoxLayout();
    profileCombo = new QComboBox(central);
    profileName = new QLineEdit(central);
    profileName->setPlaceholderText("Profile name");
    auto* loadButton = new QPushButton("Load", central);
    auto* saveButton = new QPushButton("Save profile", central);
    auto* deleteButton = new QPushButton("Delete", central);
    profileRow->addWidget(profileCombo, 1);
    profileRow->addWidget(loadButton);
    profileRow->addWidget(deleteButton);
    profileRow->addSpacing(12);
    profileRow->addWidget(profileName);
    profileRow->addWidget(saveButton);
    mainLayout->addLayout(profileRow);

    auto* toolsGroup = new QGroupBox("External tools", central);
    auto* toolsLayout = new QFormLayout(toolsGroup);
    sladePath = new QLineEdit(toolsGroup);
    builderPath = new QLineEdit(toolsGroup);
    sladePath->setPlaceholderText("Path to SLADE executable");
    builderPath->setPlaceholderText("Path to Ultimate Doom Builder executable");

    QSettings toolSettings("DoomLauncher", "DoomLauncher");
    sladePath->setText(toolSettings.value("tools/slade").toString());
    builderPath->setText(toolSettings.value("tools/ultimateDoomBuilder").toString());

    const auto persistToolPath = [](QLineEdit* pathEdit, const QString& key) {
        QSettings settings("DoomLauncher", "DoomLauncher");
        settings.setValue(key, pathEdit->text().trimmed());
        settings.sync();
        if (settings.status() != QSettings::NoError) {
            QMessageBox::critical(
                pathEdit->window(),
                "Unable to save tool path",
                "Could not save the tool path in the application settings."
            );
        }
    };

    const auto addToolRow = [this, toolsGroup, toolsLayout, persistToolPath](
        const QString& name,
        const QString& key,
        QLineEdit* pathEdit
    ) {
        auto* row = new QWidget(toolsGroup);
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        auto* browseButton = new QPushButton("Browse...", row);
        auto* launchButton = new QPushButton("Launch", row);
        rowLayout->addWidget(pathEdit, 1);
        rowLayout->addWidget(browseButton);
        rowLayout->addWidget(launchButton);
        toolsLayout->addRow(name, row);

        connect(pathEdit, &QLineEdit::editingFinished, this, [pathEdit, key, persistToolPath]() {
            persistToolPath(pathEdit, key);
        });
        connect(browseButton, &QPushButton::clicked, this, [this, pathEdit, key, name, persistToolPath]() {
            const QString path = QFileDialog::getOpenFileName(this, "Select " + name + " executable");
            if (!path.isEmpty()) {
                pathEdit->setText(path);
                persistToolPath(pathEdit, key);
            }
        });
        connect(launchButton, &QPushButton::clicked, this, [this, pathEdit, name]() {
            const QString path = pathEdit->text().trimmed();
            if (!QFileInfo(path).isFile()) {
                QMessageBox::warning(
                    this,
                    "Cannot launch " + name,
                    "Select a valid executable file first."
                );
                return;
            }

            QProcess process;
            process.setProgram(path);
            process.setWorkingDirectory(QFileInfo(path).absolutePath());
            if (!process.startDetached()) {
                QMessageBox::critical(
                    this,
                    "Unable to launch " + name,
                    process.errorString()
                );
                return;
            }
            statusBar()->showMessage("Launched " + name, 5000);
        });
    };

    addToolRow("SLADE", "tools/slade", sladePath);
    addToolRow("Ultimate Doom Builder", "tools/ultimateDoomBuilder", builderPath);
    mainLayout->addWidget(toolsGroup);

    auto* bottomRow = new QHBoxLayout();
    auto* refreshButton = new QPushButton("Refresh files", central);
    auto* launchButton = new QPushButton("Launch", central);
    bottomRow->addWidget(refreshButton);
    bottomRow->addStretch();
    bottomRow->addWidget(launchButton);
    mainLayout->addLayout(bottomRow);

    setCentralWidget(central);

    connect(browseEngine, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(this, "Select engine executable");
        if (!path.isEmpty()) {
            engineCombo->setCurrentText(path);
        }
    });
    connect(engineCombo, &QComboBox::currentTextChanged, this, [this]() {
        refreshFiles();
    });
    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        refreshFiles(true);
    });
    connect(loadButton, &QPushButton::clicked, this, [this]() {
        loadProfile();
    });
    connect(saveButton, &QPushButton::clicked, this, [this]() {
        saveProfile();
    });
    connect(deleteButton, &QPushButton::clicked, this, [this]() {
        deleteProfile();
    });
    connect(launchButton, &QPushButton::clicked, this, [this]() {
        launchGame();
    });
    refreshFiles();
    refreshProfiles();
}

QString MainWindow::modsDirectory() const {
    const QString engine = engineCombo->currentText().trimmed().toLower();
    if (engine.contains("zandronum")) {
        return QDir(doomDirectory).filePath("zand/mods");
    }
    if (engine.contains("gzdoom") || engine.contains("uzdoom")) {
        return QDir(doomDirectory).filePath("gzd/mods");
    }
    return QDir(doomDirectory).filePath("mods");
}

void MainWindow::refreshFiles(bool preserveSelection) {
    const QStringList selectedMegawads = preserveSelection ? selectedFiles(megawadList) : QStringList();
    const QStringList selectedMods = preserveSelection ? selectedFiles(modList) : QStringList();
    const QString previousIwad = iwadCombo->currentData().toString();
    iwadCombo->clear();
    for (const QString& path : matchingFiles(
             QDir(doomDirectory).filePath("iwads"), {"*.wad", "*.WAD"})) {
        iwadCombo->addItem(QFileInfo(path).fileName(), path);
    }
    const int iwadIndex = iwadCombo->findData(previousIwad);
    if (iwadIndex >= 0) {
        iwadCombo->setCurrentIndex(iwadIndex);
    }

    populateFileList(
        megawadList,
        matchingFiles(QDir(doomDirectory).filePath("megawads"), {"*.wad", "*.WAD"}, true)
    );
    populateFileList(
        modList,
        matchingFiles(modsDirectory(), {"*.wad", "*.WAD", "*.pk3", "*.PK3", "*.zip", "*.ZIP"})
    );
    if (preserveSelection) {
        setSelectedFiles(megawadList, selectedMegawads);
        setSelectedFiles(modList, selectedMods);
    }
}

void MainWindow::refreshProfiles() {
    const QString selectedProfile = profileCombo->currentData().toString();
    profileCombo->clear();
    const QString profileDirectory = QDir(doomDirectory).filePath("profiles");
    const QStringList files = QDir(profileDirectory).entryList({"*.json"}, QDir::Files, QDir::Name);
    for (const QString& file : files) {
        const QString name = QFileInfo(file).completeBaseName();
        profileCombo->addItem(name, QDir(profileDirectory).filePath(file));
    }
    const int index = profileCombo->findData(selectedProfile);
    if (index >= 0) {
        profileCombo->setCurrentIndex(index);
    }
}

QStringList MainWindow::selectedFiles(const QListWidget* list) const {
    QStringList paths;
    for (int index = 0; index < list->count(); ++index) {
        const QListWidgetItem* item = list->item(index);
        if (item->isSelected()) {
            paths.append(item->text());
        }
    }
    return paths;
}

void MainWindow::setSelectedFiles(QListWidget* list, const QStringList& paths) {
    list->clearSelection();
    QStringList missing;
    QList<QListWidgetItem*> orderedItems;
    for (const QString& path : paths) {
        bool found = false;
        for (int index = 0; index < list->count(); ++index) {
            QListWidgetItem* item = list->item(index);
            if (item->text() == path) {
                item = list->takeItem(index);
                item->setSelected(true);
                orderedItems.append(item);
                found = true;
                break;
            }
        }
        if (!found && QFileInfo::exists(path)) {
            auto* item = new QListWidgetItem(path);
            item->setSelected(true);
            orderedItems.append(item);
        } else if (!found) {
            missing.append(path);
        }
    }
    for (int index = 0; index < orderedItems.size(); ++index) {
        list->insertItem(index, orderedItems.at(index));
    }
    if (!missing.isEmpty()) {
        QMessageBox::warning(
            this,
            "Files missing from profile",
            "These profile entries could not be found:\n" + missing.join('\n')
        );
    }
}

void MainWindow::loadProfile() {
    const QString path = profileCombo->currentData().toString();
    if (path.isEmpty()) {
        QMessageBox::information(this, "Load profile", "No saved profiles are available.");
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Unable to load profile", file.errorString());
        return;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, "Unable to load profile", parseError.errorString());
        return;
    }
    if (!document.isObject()) {
        QMessageBox::critical(this, "Unable to load profile", "The profile must contain a JSON object.");
        return;
    }

    const QJsonObject profile = document.object();
    engineCombo->setCurrentText(profile.value("engine").toString());
    refreshFiles();
    const QString iwad = profile.value("iwad").toString();
    int iwadIndex = iwadCombo->findData(iwad);
    if (iwadIndex < 0 && QFileInfo::exists(iwad)) {
        iwadCombo->addItem(QFileInfo(iwad).fileName(), iwad);
        iwadIndex = iwadCombo->count() - 1;
    }
    if (iwadIndex >= 0) {
        iwadCombo->setCurrentIndex(iwadIndex);
    } else if (!iwad.isEmpty()) {
        QMessageBox::warning(this, "IWAD missing", "The IWAD in this profile could not be found:\n" + iwad);
    }
    setSelectedFiles(megawadList, fromJsonArray(profile.value("megawads")));
    setSelectedFiles(modList, fromJsonArray(profile.value("mods")));
    profileName->setText(profileCombo->currentText());
    statusBar()->showMessage("Profile loaded", 4000);
}

void MainWindow::saveProfile() {
    const QString name = profileName->text().trimmed();
    static const QRegularExpression validName("^[A-Za-z0-9][A-Za-z0-9 _.-]*$");
    if (!validName.match(name).hasMatch()) {
        QMessageBox::warning(
            this,
            "Invalid profile name",
            "Use a name that starts with a letter or number and contains only letters, numbers, spaces, dots, underscores, or hyphens."
        );
        return;
    }
    const QString engine = engineCombo->currentText().trimmed();
    const QString iwad = iwadCombo->currentData().toString();
    if (engine.isEmpty() || iwad.isEmpty()) {
        QMessageBox::warning(this, "Cannot save profile", "Choose an engine and an IWAD before saving a profile.");
        return;
    }

    const QString path = QDir(doomDirectory).filePath("profiles/" + name + ".json");
    if (QFileInfo::exists(path)) {
        const auto answer = QMessageBox::question(
            this, "Replace profile", "A profile with this name already exists. Replace it?"
        );
        if (answer != QMessageBox::Yes) {
            return;
        }
    }

    QJsonObject profile;
    profile.insert("engine", engine);
    profile.insert("iwad", iwad);
    profile.insert("megawads", toJsonArray(selectedFiles(megawadList)));
    profile.insert("mods", toJsonArray(selectedFiles(modList)));

    QSaveFile file(path);
    const QByteArray content = QJsonDocument(profile).toJson(QJsonDocument::Indented);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Unable to save profile", file.errorString());
        return;
    }
    if (file.write(content) != content.size() || !file.commit()) {
        QMessageBox::critical(this, "Unable to save profile", file.errorString());
        return;
    }
    refreshProfiles();
    const int index = profileCombo->findData(path);
    if (index >= 0) {
        profileCombo->setCurrentIndex(index);
    }
    statusBar()->showMessage("Profile saved", 4000);
}

void MainWindow::deleteProfile() {
    const QString path = profileCombo->currentData().toString();
    if (path.isEmpty()) {
        QMessageBox::information(this, "Delete profile", "Select a saved profile first.");
        return;
    }
    if (QMessageBox::question(
            this, "Delete profile", "Delete profile \"" + profileCombo->currentText() + "\"?"
        ) != QMessageBox::Yes) {
        return;
    }
    if (!QFile::remove(path)) {
        QMessageBox::critical(this, "Unable to delete profile", "Could not remove " + path);
        return;
    }
    refreshProfiles();
    statusBar()->showMessage("Profile deleted", 4000);
}

void MainWindow::launchGame() {
    const QString engine = engineCombo->currentText().trimmed();
    const QString iwad = iwadCombo->currentData().toString();
    if (engine.isEmpty() || iwad.isEmpty() || !QFileInfo(iwad).isFile()) {
        QMessageBox::warning(this, "Cannot launch", "Choose an engine and a valid IWAD first.");
        return;
    }

    QStringList files = selectedFiles(megawadList);
    files.append(selectedFiles(modList));
    for (const QString& path : files) {
        if (!QFileInfo(path).isFile()) {
            QMessageBox::warning(this, "Cannot launch", "Selected file could not be found:\n" + path);
            return;
        }
    }
    QStringList arguments = {"-iwad", iwad};
    if (!files.isEmpty()) {
        arguments.append("-file");
        arguments.append(files);
    }

    gameProcess.setProgram(engine);
    gameProcess.setArguments(arguments);
    if (!gameProcess.startDetached()) {
        QMessageBox::critical(this, "Unable to launch game", gameProcess.errorString());
        statusBar()->showMessage("Launch failed");
        return;
    }
    statusBar()->showMessage("Launched " + engine, 5000);
}
