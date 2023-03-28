#include "cmainwindow.h"
#include "ui_cmainwindow.h"

#include "csubsonic.h"

#include <QDateTime>


cMainWindow::cMainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::cMainWindow)
{
	ui->setupUi(this);

	cSubsonic*					subsonic	= new cSubsonic("https://music.windesign.at", "birkeh", "WeissIchNicht8");

	bool						licenseValid;
	QString						licenseEmail;
	QDateTime					licenseExpires;
	QMap<qint32, QString>		musicFolders;
	QMap<qint32, QString>		shortcuts;
	QStringList					indexes;
	QMap<qint32, INDEX>			indexArtists;

	QString						artistName;
	qint32						artistPlayCount;
	QMap<qint32, ALBUM>			albums;

	qint32						albumParent;
	QString						albumName;
	qint32						albumPlayCount;
	QMap<qint32, TRACK>			tracks;

	QMap<QString, GENRE>		genres;

	QMap<qint32, ARTIST>		artists;

	subsonic->getLicense(licenseValid, licenseEmail, licenseExpires);
	subsonic->getMusicFolders(musicFolders);
	subsonic->getIndexes(shortcuts, indexes, indexArtists);
	subsonic->getMusicDirectory(230, artistName, artistPlayCount, albums);
	subsonic->getMusicDirectory(885, albumParent, albumName, albumPlayCount, tracks);
	subsonic->getGenres(genres);
	subsonic->getArtists(indexes, artists);

	delete subsonic;
}

cMainWindow::~cMainWindow()
{
	delete ui;
}
