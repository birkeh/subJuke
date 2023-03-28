#ifndef CSUBSONIC_H
#define CSUBSONIC_H


#include <QObject>
#include <QDomElement>
#include <QMap>
#include <QDate>


typedef struct tagINDEX
{
	QString		index;
	QString		name;
	qint32		playCount;
} INDEX;

typedef struct tagALBUM
{
	qint32		parent;
	QString		title;
	QString		album;
	QString		artist;
	qint16		year;
	QString		genre;
	qint32		coverArt;
	qint32		playCount;
	QDateTime	created;
} ALBUM;

typedef struct tagTRACK
{
	qint32		parent;
	QString		title;
	QString		album;
	QString		artist;
	QString		track;
	qint16		year;
	QString		genre;
	qint32		coverArt;
	qint32		size;
	QString		contentType;
	QString		suffix;
	qint32		duration;
	qint32		bitRate;
	QString		path;
	bool		isVideo;
	qint32		playCount;
	QString		discNumber;
	QDateTime	created;
	qint32		albumID;
	qint32		artistID;
	QString		type;
} TRACK;

typedef struct tagGENRE
{
	qint32		songCount;
	qint32		albumCount;
} GENRE;

typedef struct tagARTIST
{
	QString		index;
	QString		name;
	QString		coverArt;
	qint32		albumCount;
} ARTIST;

class cSubsonic
{
public:
	cSubsonic();
	cSubsonic(const QString& server, const QString& user, const QString& password);

	void		setServer(const QString& server);
	QString		server();

	void		setUser(const QString& user);
	QString		user();

	void		setPassword(const QString& password);

	bool		verify();
	bool		getLicense(bool &valid, QString &email, QDateTime &licenseExpires);
	bool		getMusicFolders(QMap<qint32, QString>& folders);
	bool		getIndexes(QMap<qint32, QString>& shortcuts, QStringList& indexes, QMap<qint32, INDEX>& artists, const qint32 id = -1, const QDate& modifiedSince = QDate());
	bool		getMusicDirectory(const qint32 id, QString& name, qint32& playCount, QMap<qint32, ALBUM>& albums);
	bool		getMusicDirectory(const qint32 id, qint32& parent, QString& name, qint32& playCount, QMap<qint32, TRACK>& tracks);
	bool		getGenres(QMap<QString, GENRE>& genres);
	bool		getArtists(QStringList& indexes, QMap<qint32, ARTIST>& artists, const qint32 id = -1);
private:
	QString		m_server;
	QString		m_user;
	QString		m_password;
	QString		m_version;

	QString		randomString();
	bool		sendRequest(const QString& request, QDomElement& element, const QString& verify = "", const QMap<QString, QString>& parameter = QMap<QString, QString>());
};

#endif // CSUBSONIC_H
