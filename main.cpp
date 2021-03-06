/*
 *
 * PhotoQt is a simple image viewer, designed to be good looking,
 * highly configurable, yet easy to use and fast.
 *
 * Copyright (C) 2013, Lukas Spies (Lukas@photoqt.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <QtGui>
#include "mainwindow.h"
#include <iostream>
#include <QTranslator>
#include <string>

int main(int argc, char *argv[]) {

	QApplication::setApplicationName("photoqt");

	// This string holds the current version
	QString globVersion = QString::fromStdString(VERSION);

	// A help message for the command line
	QString hlp = "\nPhotoQt v" + globVersion + " -  Copyright (C) 2013, Lukas Spies (Lukas@photoqt.org), License: GPLv2 (or later)\n";
	hlp += "PhotoQt is a fast, simple, good looking, yet powerfull and highly configurable image viewer.\n\n";

	hlp += "Usage: photoqt [options|file]\n\n";

	hlp += "Options:\n";
	hlp += "\t--h, --help\t\tThis help message\n\n";

	hlp += ">> Start-up options:\n\n";

	hlp += "\t--start-in-tray\t\tStart PhotoQt hidden to the system tray\n";
	hlp += "\t--no-thumbs\t\tDon't load thumbnails (Navigation through folder is still possible)\n\n";

	hlp += ">> Remote Controlling:\n\n";

	hlp += "\t--open\t\t\tOpens the open file dialog (also shows PhotoQt if hidden)\n";
	hlp += "\t--toggle\t\tToggles PhotoQt - hides PhotoQt if visible, shows if hidden\n";
	hlp += "\t--show\t\t\tShows PhotoQt (does nothing if already shown)\n";
	hlp += "\t--hide\t\t\tHides PhotoQt (does nothing if already hidden)\n\n";

	hlp += ">> Remote controlling w/ filename needed:\n\n";

	hlp += "\t--no-thumbs [filename]\tDon't load thumbnails (Navigation through folder is still possible)\n";
	hlp += "\t--thumbs [filename]\tReversing a '--no-thumbs' (thumbnails are enabled by default)\n\n";

	hlp += ">> Debuging:\n\n";
	hlp += "\t--v, --verbose\t\tEnabling debug messages\n\n";

	hlp += "\n   Enjoy PhotoQt :-)\n\n\n";

	// This file is updated by a running instance of PhotoQt every 500 milliseconds - check
	QFile chk(QDir::homePath() + "/.photoqt/running");
	QString all = "";
	if(chk.open(QIODevice::ReadOnly)) {
		QTextStream in(&chk);
		all = in.readAll();
	}

	QStringList allArgs;
	for(int i = 0; i < argc; ++i)
		allArgs.append(argv[i]);

	if(QFile(QDir::homePath()+"/.photoqt/verbose").exists())
		allArgs.append("--v");

	QStringList knownArgs;
	knownArgs << "--open";
	knownArgs << "--no-thumbs";
	knownArgs << "--thumbs";
	knownArgs << "--toggle";
	knownArgs << "--show";
	knownArgs << "--hide";
	knownArgs << "--start-in-tray";
	knownArgs << "--verbose";
	knownArgs << "--v";

	/****************************************************/
	// DEVELOPMENT ONLY
	knownArgs << "--update" << "--install";
	// DEVELOPMENT ONLY
	/****************************************************/

	// If PhotoQt was started with "--h" or "--help", show help message
	if(allArgs.contains("--help") || allArgs.contains("-help") || allArgs.contains("--h") || allArgs.contains("-h")) {

		std::cout << hlp.toStdString();

		return 0;

	// If an instance of PhotoQt is running, we check for command line arguments
	} else if(QDateTime::currentMSecsSinceEpoch() - all.toLongLong() < qint64(1020)) {

		// We need to initiate it here to, because we check for the applicationFilePath() later-on
		QApplication a(argc, argv);

		std::cout << "Running instance of PhotoQt detected..." << std::endl;

		// This is the content of the file used to communicate with running PhotoQt instance
		QString cont = "";

		// This boolean is set to true if an unknown command is used
		bool err = false;

		for(int i = 1; i < allArgs.length(); ++i) {

			// We ignore the verbose switch when an instance is already running
			if(allArgs.at(i) != "--v" && allArgs.at(i) != "--verbose") {
				if(knownArgs.contains(allArgs.at(i)))
					cont += allArgs.at(i) + "\n";
				else if(allArgs.at(i).startsWith("-")) {
					err = true;
				} else {
					QString filename = allArgs.at(i);
					if(!filename.startsWith("/"))
						filename = QFileInfo(filename).absoluteFilePath();
					if(filename != a.applicationFilePath())
						cont += "-f-" + filename;
				}
			}

		}

		// If PhotoQt is called without any arguments, "show" is used
		if(allArgs.length() == 1 || (allArgs.length() == 2 && allArgs.contains("--v")))
			cont = "--show";

		// If only correct arguments were used
		if(!err) {
			// Write the commands into this file, which is checked regularly by a running instance of PhotoQt
			QFile f(QDir::homePath() + "/.photoqt/cmd");
			f.remove();
			if(f.open(QIODevice::WriteOnly)) {
				QTextStream out(&f);
				out << cont;
				f.close();
			} else
				std::cerr << "ERROR! Couldn't write to file '~/.photoqt/cmd'. Unable to communicate with running process" << std::endl;

		// If an uncorrect argument was used
		} else
			std::cout << hlp.toStdString();


		return 0;

	// If PhotoQt isn't running and no command line argument (besides filename and "--start-in.tray") was used
	} else {

		bool verbose = (allArgs.contains("--v") || allArgs.contains("--verbose") || QFile(QDir::homePath() + "/.photoqt/verbose").exists() || (!QDir(QDir::homePath() + "/.photoqt").exists() && QFile(QDir::homePath() + "/.photo/verbose").exists()));

		bool migrated = false;

		// Ensure that the config folder exists, and move from ~/.photo to ~/.photoqt
		QDir dir(QDir::homePath() + "/.photoqt");
		if(!dir.exists()) {
			QDir dir_old(QDir::homePath() + "/.photo");
			if(dir_old.exists()) {
				if(verbose) std::clog << "Moving ~/.photo to ~/.photoqt" << std::endl;
				dir.mkdir(QDir::homePath() + "/.photoqt");

				QFile file(QDir::homePath() + "/.photo/contextmenu");
				if(file.exists()) {
					file.copy(QDir::homePath() + "/.photoqt/contextmenu");
					file.remove();
				}

				file.setFileName(QDir::homePath() + "/.photo/fileformats");
				if(file.exists()) {
					file.copy(QDir::homePath() + "/.photoqt/fileformats");
					file.remove();
				}

				file.setFileName(QDir::homePath() + "/.photo/running");
				if(file.exists()) {
					file.copy(QDir::homePath() + "/.photoqt/running");
					file.remove();
				}

				file.setFileName(QDir::homePath() + "/.photo/settings");
				if(file.exists()) {
					file.copy(QDir::homePath() + "/.photoqt/settings");
					file.remove();
				}

				file.setFileName(QDir::homePath() + "/.photo/shortcuts");
				if(file.exists()) {
					file.copy(QDir::homePath() + "/.photoqt/shortcuts");
					file.remove();
				}

				file.setFileName(QDir::homePath() + "/.photo/thumbnails");
				if(file.exists()) {
					file.copy(QDir::homePath() + "/.photoqt/thumbnails");
					file.remove();
				}

				file.setFileName(QDir::homePath() + "/.photo/verbose");
				if(file.exists()) {
					file.copy(QDir::homePath() + "/.photoqt/verbose");
					file.remove();
				}

				dir_old.rmdir(dir_old.absolutePath());

				migrated = true;


			} else {
				if(verbose) std::clog << "Creating ~/.photoqt/" << std::endl;
				dir.mkdir(QDir::homePath() + "/.photoqt");
			}
		}

		bool err = false;

		for(int i = 0; i < allArgs.length(); ++i) {
			if(allArgs.at(i).startsWith("-") && !knownArgs.contains(allArgs.at(i)))
				err = true;
		}

		if(err == true) {

			std::cout << hlp.toStdString();

			// Nothing after this return will be executed (PhotoQt will simply quit)
			return 0;
		}

		// This int holds 1 if PhotoQt was updated and 2 if it's newly installed
		int update = 0;
		QString settingsFileTxt = "";

		// Check if the settings file exists. If not, create a file with default settings (i.e. empty file, settings and defaults are handled by globalsettings.h)
		QFile file(QDir::homePath() + "/.photoqt/settings");
		if(!file.exists()) {
			if(!file.open(QIODevice::WriteOnly))
				std::cerr << "ERROR: Couldn't write settings file! Please ensure that you have read&write access to home directory" << std::endl;
			else {
				if(verbose) std::clog << "Creating basic settings file" << std::endl;
				QTextStream out(&file);
				out << "Version=" + globVersion + "\n";
				file.close();
			}

			update = 2;

		// If file does exist, check if it is from a previous version -> PhotoQt was updated
		} else {
			if(!file.open(QIODevice::ReadWrite))
				std::cerr << "ERROR: Couldn't read settings file! Please ensure that you have read&write access to home directory" << std::endl;
			else {
				QTextStream in(&file);
				settingsFileTxt = in.readAll();

				if(verbose) std::clog << "Checking if first run of new version" << std::endl;

				// If it doesn't contain current version (some previous version)
				if(!settingsFileTxt.contains("Version=" + globVersion + "\n")) {
					file.close();
					file.remove();
					file.open(QIODevice::ReadWrite);
					QStringList allSplit = settingsFileTxt.split("\n");
					allSplit.removeFirst();
					QString allFile = "Version=" + globVersion + "\n" + allSplit.join("\n");
					in << allFile;
					update = 1;
				}

				file.close();

			}
		}

		/****************************************************/
		// DEVELOPMENT ONLY
		if(allArgs.contains("--update")) update = 1;
		if(allArgs.contains("--install")) update = 2;
		// DEVELOPMENT ONLY
		/****************************************************/

#ifdef GM
		Magick::InitializeMagick(*argv);
#endif

		if(QFile(QDir::homePath()+"/.photoqt/cmd").exists())
			QFile(QDir::homePath()+"/.photoqt/cmd").remove();

		// This boolean stores if PhotoQt needs to be minimized to the tray
		bool startintray = allArgs.contains("--start-in-tray");

		// If PhotoQt is supposed to be started minimized in system tray
		if(startintray) {
			if(verbose) std::clog << "Starting minimised to tray" << std::endl;
			// If the option "Use Tray Icon" in the settings is not set, we set it
			QFile set(QDir::homePath() + "/.photoqt/settings");
			if(set.open(QIODevice::ReadOnly)) {
				QTextStream in(&set);
				QString all = in.readAll();
				if(!all.contains("TrayIcon=1")) {
					if(all.contains("TrayIcon=0"))
						all.replace("TrayIcon=0","TrayIcon=1");
					else
						all += "\n[Temporary Appended]\nTrayIcon=1\n";
					set.close();
					set.remove();
					if(!set.open(QIODevice::WriteOnly))
						std::cerr << "ERROR: Can't enable tray icon setting!" << std::endl;
					QTextStream out(&set);
					out << all;
					set.close();
				} else
					set.close();
			} else
				std::cerr << "Unable to ensure TrayIcon is enabled - make sure it is enabled!!" << std::endl;
		}

		QApplication a(argc, argv);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
		// Opt-in to High DPI usage of Pixmaps for larger screens with larger font DPI
		a.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

		// LOAD THE TRANSLATOR
		QTranslator trans;

		// We use two strings, since the system locale usually is of the form e.g. "de_DE"
		// and some translations only come with the first part, i.e. "de",
		// and some with the full string. We need to be able to find both!
		if(verbose) std::clog << "Checking for translation" << std::endl;
		QString code1 = "";
		QString code2 = "";
		if(settingsFileTxt.contains("Language=") && !settingsFileTxt.contains("Language=en") && !settingsFileTxt.contains("Language=\n")) {
			code1 = settingsFileTxt.split("Language=").at(1).split("\n").at(0).trimmed();
			code2 = code1;
		} else if(!settingsFileTxt.contains("Language=en")) {
			code1 = QLocale::system().name();
			code2 = QLocale::system().name().split("_").at(0);
		}
		if(verbose) std::clog << "Found following language: " << code1.toStdString()  << "/" << code2.toStdString() << std::endl;
		if(QFile(":/lang/photoqt_" + code1 + ".qm").exists()) {
			std::clog << "Loading Translation:" << code1.toStdString() << std::endl;
			trans.load(":/lang/photoqt_" + code1);
			a.installTranslator(&trans);
			code2 = code1;
		} else if(QFile(":/lang/photoqt_" + code2 + ".qm").exists()) {
			std::clog << "Loading Translation:" << code2.toStdString() << std::endl;
			trans.load(":/lang/photoqt_" + code2);
			a.installTranslator(&trans);
			code1 = code2;
		}

		// Check if thumbnail database exists. If not, create it
		QFile database(QDir::homePath() + "/.photoqt/thumbnails");
		if(!database.exists()) {

			if(verbose) std::clog << "Create Thumbnail Database" << std::endl;

			QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "thumbDB");
			db.setDatabaseName(QDir::homePath() + "/.photoqt/thumbnails");
			if(!db.open()) std::cerr << "ERROR: Couldn't open thumbnail database:" << db.lastError().text().trimmed().toStdString() << std::endl;
			QSqlQuery query(db);
			query.prepare("CREATE TABLE Thumbnails (filepath TEXT,thumbnail BLOB, filelastmod INT, thumbcreated INT, origwidth INT, origheight INT)");
			query.exec();
			if(query.lastError().text().trimmed().length()) std::cerr << "ERROR (Creating Thumbnail Datbase):" << query.lastError().text().trimmed().toStdString() << std::endl;
			query.clear();


		} else {

			if(verbose) std::clog << "Opening Thumbnail Database" << std::endl;

			// Opening the thumbnail database
			QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","thumbDB");
			db.setDatabaseName(QDir::homePath() + "/.photoqt/thumbnails");
			if(!db.open()) std::cerr << "ERROR: Couldn't open thumbnail database:" << db.lastError().text().trimmed().toStdString() << std::endl;

			if(migrated) {
				QSqlQuery query(db);
				query.prepare("ALTER TABLE Thumbnails ADD COLUMN origwidth INT");
				query.exec();
				if(query.lastError().text().trimmed().length()) std::cerr << "ERROR (Adding origwidth to Thumbnail Database):" << query.lastError().text().trimmed().toStdString() << std::endl;
				query.clear();
				query.prepare("ALTER TABLE Thumbnails ADD COLUMN origheight INT");
				query.exec();
				if(query.lastError().text().trimmed().length()) std::cerr << "ERROR (Adding origheight to Thumbnail Database):" << query.lastError().text().trimmed().toStdString() << std::endl;
				query.clear();
			}

		}


		// Previous versions of PhotoQt checked here also for the shortcuts file. We don't need to do that anymore, since all shortcuts including the defaults are handled by shortcuts.h


		/***************************
		 ***************************/
		// The Window has to be initialised *AFTER* the checks above to ensure that the settings exist and are updated and can be loaded
		MainWindow w(0,verbose);
		/***************************
		 ***************************/

		// We move from old way of handling image formats to new way
		// We can't do it before here, since we need access to global settings
		QFile old_qt(QDir::homePath() + "/.photoqt/fileformatsQt");
		QFile old_gm(QDir::homePath() + "/.photoqt/fileformatsGm");
		QFile new_file(QDir::homePath() + "/.photoqt/fileformats.disabled");
		if((old_qt.exists() || old_gm.exists()) && !new_file.exists()) {

			// These will be the sets of known file formats
			QStringList new_qt;
			QStringList new_gm;

			// Read in formats supported by qt
			if(old_qt.exists()) {
				if(!old_qt.open(QIODevice::ReadOnly))
					std::cerr << "[migrate fileformats] ERROR: Can't open old file with qt file formats";
				else {
					QTextStream in(&old_qt);
					QString line = in.readLine();
					while (!line.isNull()) {
						if(line.trimmed().length() != 0) new_qt << "*" + line.trimmed();
						line = in.readLine();
					}
					old_qt.close();
				}
				// Remove old and redundant file
				if(!old_qt.remove())
					std::cerr << "[migrate fileformats] WARNING: Can't remove old (redundant) file with qt file formats";
			}

			// Read in formats supported by gm
			if(old_gm.exists()) {
				if(!old_gm.open(QIODevice::ReadOnly))
					std::cerr << "[migrate fileformats] ERROR: Can't open old file with gm file formats";
				else {
					QTextStream in(&old_gm);
					QString line = in.readLine();
					while (!line.isNull()) {
						if(line.trimmed().length() != 0) new_gm << "*" + line.trimmed();
						line = in.readLine();
					}
					old_gm.close();
				}
				// Remove old and redundant file
				if(!old_gm.remove())
					std::cerr << "[migrate fileformats] WARNING: Can't remove old (redundant) file with gm file formats";
			}

			// File content of disabled fileformats
			QString fileformatsDisabled = "";

			// New fileformats that were not part of previous versions of PhotoQt
			QStringList newfileformats;
			newfileformats << "*.cin";
			newfileformats << "*.mono";
			newfileformats << "*.sfw";
			newfileformats << "*.txt";
			newfileformats << "*.wpg";

			// QT: If not supported, it is disabled
			QStringList qtDef;
			qtDef << "*.bmp, *.bitmap"
				<< "*.dds"
				<< "*.gif"
				<< "*.ico, *.icns"
				<< "*.jpg, *.jpeg"
				<< "*.jpeg2000, *.jp2, *.jpc, *.j2k, *.jpf, *.jpx, *.jpm, *.mj2"
				<< "*.mng"
				<< "*.png"
				<< "*.pbm"
				<< "*.pgm"
				<< "*.ppm"
				<< "*.svg, *.svgz"
				<< "*.tif, *.tiff"
				<< "*.wbmp, *.webp"
				<< "*.xbm"
				<< "*.xpm";
			QList<QByteArray> qtSup = QImageReader::supportedImageFormats();
			foreach(QString q, qtDef) {
				QStringList parts = q.split(", ");
				bool disabled = true;
				foreach(QString p, parts) {
					if(qtSup.contains(p.remove(0,2).toLower().trimmed().toLatin1())) {
						disabled = false;
						break;
					}
				}
				if(disabled)
					fileformatsDisabled += parts.join("\n") + "\n";
			}

			// GM: If not enabled, it is disabled - new fileformats are ENabled by default
			QStringList gmDef = w.globSet->fileFormats->formatsGmEnabled;
			foreach(QString g, gmDef) {
				if(!new_gm.contains(g) && !newfileformats.contains(g))
					fileformatsDisabled += g + "\n";
			}

			// Extras are disabled by default
			fileformatsDisabled += "**.xcf\n**.psd\n**.psb\n";

			// Untested are disabled by default
			if(!fileformatsDisabled.contains("*.hp\n")) fileformatsDisabled += "*.hp\n";
			if(!fileformatsDisabled.contains("*.hpgl\n")) fileformatsDisabled += "*.hpgl\n";
			if(!fileformatsDisabled.contains("*.jbig\n")) fileformatsDisabled += "*.jbig\n";
			if(!fileformatsDisabled.contains("*.jbg\n")) fileformatsDisabled += "*.jbg\n";
			if(!fileformatsDisabled.contains("*.pwp\n")) fileformatsDisabled += "*.pwp\n";
			if(!fileformatsDisabled.contains("*.rast\n")) fileformatsDisabled += "*.rast\n";
			if(!fileformatsDisabled.contains("*.rla\n")) fileformatsDisabled += "*.rla\n";
			if(!fileformatsDisabled.contains("*.rle\n")) fileformatsDisabled += "*.rle\n";
			if(!fileformatsDisabled.contains("*.sct\n")) fileformatsDisabled += "*.sct\n";
			if(!fileformatsDisabled.contains("*.tim\n")) fileformatsDisabled += "*.tim\n";

			// Write 'disabled filetypes' file
			if(new_file.open(QIODevice::WriteOnly)) {
				QTextStream out(&new_file);
				out << fileformatsDisabled;
				new_file.close();
			} else
				std::cerr << "ERROR: Can't write default disabled fileformats file" << std::endl;


			// Update settings with new values
			w.globSet->fileFormats->getFormats();

		}

		// DISPLAY MAINWINDOW
		if(!startintray) {
			bool keepOnTop = settingsFileTxt.contains("KeepOnTop=1");
			if(settingsFileTxt.contains("WindowMode=1")) {
				if(keepOnTop) {
					settingsFileTxt.contains("WindowDecoration=1")
						      ? w.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint)
						      : w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
				} else {
					settingsFileTxt.contains("WindowDecoration=1")
						      ? w.setWindowFlags(Qt::Window)
						      : w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
				}

				QSettings settings;
				if(settings.allKeys().contains("mainWindowGeometry") && settingsFileTxt.contains("SaveWindowGeometry=1")) {
					w.show();
					w.restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
				} else
					w.showMaximized();

			} else {
				if(keepOnTop) w.setWindowFlags(Qt::WindowStaysOnTopHint);
				w.showFullScreen();
			}
		} else
			w.hide();

		QString file_str = "";

		// Look for a filename possibly passed on by the user. We NEED to start at i=1, otherwise it simply takes the app name as passed on filename
		for(int i = 1; i < allArgs.length(); ++i) {
			if(!allArgs.at(i).startsWith("-")) {
				QString filename = allArgs.at(i);
				filename = QFileInfo(filename).absoluteFilePath();
				if(filename != QApplication::applicationFilePath()) {
					if(verbose) std::clog << "Filename submitted:" << filename.toStdString() << std::endl;
					file_str = filename;
					break;
				}
			}
		}


		// Possibly disable thumbnails
		if(allArgs.contains("--no-thumbs")) {
			if(verbose) std::clog << "Disabling Thumbnails" << std::endl;
			QMap<QString,QVariant> upd;
			upd.insert("ThumbnailDisable",true);
			w.globSet->settingsUpdated(upd);
		}


		// Set the file to open that the user might have passed on
		w.globVar->currentfile = file_str;

		w.globVar->startupMessageInstallUpdateShown = update;

		if(!startintray)
			w.startUpTimer->start();

		return a.exec();

	}
}



