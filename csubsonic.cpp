#include "csubsonic.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>

#include <QDomDocument>

#include <QCryptographicHash>

#include <QDateTime>

#include <QDebug>


cSubsonic::cSubsonic() :
	m_server(""),
	m_user(""),
	m_password(""),
	m_version("1.15.0")
{
	verify();
}

cSubsonic::cSubsonic(const QString& server, const QString& user, const QString& password) :
	m_server(server),
	m_user(user),
	m_password(password),
	m_version("1.15.0")
{
	verify();
}

void cSubsonic::setServer(const QString& server)
{
	m_server	= server;
}

QString cSubsonic::server()
{
	return(m_server);
}

void cSubsonic::setUser(const QString& user)
{
	m_user	= user;
}

QString cSubsonic::user()
{
	return(m_user);
}

void cSubsonic::setPassword(const QString& password)
{
	m_password	= password;
}

bool cSubsonic::verify()
{
	QDomElement			element;

	if(!sendRequest("ping.view", element))
	{
		m_version	= element.attribute("version");

		if(!sendRequest("ping.view", element))
			return(false);
	}

	return(true);
}

bool cSubsonic::getLicense(bool& valid, QString& email, QDateTime& licenseExpires)
{
	QDomElement			element;

	if(!sendRequest("getLicense", element, "license"))
		return(false);

	valid			= element.attribute("valid") == "true";
	email			= element.attribute("email");
	licenseExpires	= QDateTime::fromString(element.attribute("licenseExpires"), "yyyy-MM-ddThh:mm:ss.zzzZ");

	return(true);
}

bool cSubsonic::getMusicFolders(QMap<qint32, QString>& folders)
{
	QDomElement			element;

	if(!sendRequest("getMusicFolders", element, "musicFolders"))
		return(false);

	QDomNode			child	= element.firstChild();
	QDomElement			childElement;
	qint32				id;
	QString				name;

	while(!child.isNull())
	{
		childElement	= child.toElement();

		if(!childElement.tagName().compare("musicFolder"))
		{
			id		= childElement.attribute("id").toInt();
			name	= childElement.attribute("name");
			folders.insert(id, name);
		}
		child	= child.nextSibling();
	}

	return(true);
}

bool cSubsonic::getIndexes(QMap<qint32, QString>& shortcuts, QStringList& indexes, QMap<qint32, INDEX> &artists, const qint32 id, const QDate& modifiedSince)
{
	QMap<QString, QString>	parameter;
	QDomElement				element;

	if(id != -1)
		parameter.insert("musicFolderId", QString::number(id));

	if(modifiedSince.isValid())
		parameter.insert("ifModifiedSince", QString::number(QDateTime(modifiedSince, QTime(0, 0)).toMSecsSinceEpoch()));

	if(!sendRequest("getIndexes", element, "indexes", parameter))
		return(false);

	QDomNode			child	= element.firstChild();
	QDomElement			childElement;
	QDomElement			artistElement;

	qint32				shortcutID;
	QString				shortcutName;
	QString				indexName;
	qint32				artistID;
	QString				artistName;

	while(!child.isNull())
	{
		childElement	= child.toElement();

		if(!childElement.tagName().compare("shortcut"))
		{
			shortcutID		= childElement.attribute("id").toInt();
			shortcutName	= childElement.attribute("name");
			shortcuts.insert(shortcutID, shortcutName);
		}
		else if(!childElement.tagName().compare("index"))
		{
			indexName		= childElement.attribute("name");
			indexes.append(indexName);

			QDomNode	artistChild	= childElement.firstChild();

			while(!artistChild.isNull())
			{
				artistElement	= artistChild.toElement();

				if(!artistElement.tagName().compare("artist"))
				{
					artistID	= artistElement.attribute("id").toInt();
					artistName	= artistElement.attribute("name");

					INDEX	artist	= { indexName, artistName, -1 };
					artists.insert(artistID, artist);
				}

				artistChild	= artistChild.nextSibling();
			}
		}

		child	= child.nextSibling();
	}

	return(true);
}

bool cSubsonic::getMusicDirectory(const qint32 id, QString &name, qint32 &playCount, QMap<qint32, ALBUM>& albums)
{
	QMap<QString, QString>	parameter;
	QDomElement				element;

	parameter.insert("id", QString::number(id));

	if(!sendRequest("getMusicDirectory", element, "directory", parameter))
		return(false);

	name			= element.attribute("name");
	playCount		= element.attribute("playCount").toInt();

	QDomNode			child	= element.firstChild();
	QDomElement			childElement;
	QDomElement			artistElement;

	while(!child.isNull())
	{
		childElement	= child.toElement();

		if(!childElement.tagName().compare("child"))
		{
			if(!childElement.hasAttribute("contentType"))
			{
				ALBUM	album;
				qint32	albumID;

				albumID			= childElement.attribute("id").toInt();
				album.parent	= childElement.attribute("parent").toInt();
				album.title		= childElement.attribute("title");
				album.album		= childElement.attribute("album");
				album.artist	= childElement.attribute("artist");
				album.year		= childElement.attribute("year").toInt();
				album.genre		= childElement.attribute("genre");
				album.coverArt	= childElement.attribute("coverArt").toInt();
				album.playCount	= childElement.attribute("playCount").toInt();
				album.created	= QDateTime::fromString(childElement.attribute("created"), "yyyy-MM-ddThh:mm:ss.zzzZ");

				albums.insert(albumID, album);
			}
		}

		child	= child.nextSibling();
	}

	return(true);
}

bool cSubsonic::getMusicDirectory(const qint32 id, qint32& parent, QString& name, qint32& playCount, QMap<qint32, TRACK>& tracks)
{
	QMap<QString, QString>	parameter;
	QDomElement				element;

	parameter.insert("id", QString::number(id));

	if(!sendRequest("getMusicDirectory", element, "directory", parameter))
		return(false);

	parent			= element.attribute("parent").toInt();
	name			= element.attribute("name");
	playCount		= element.attribute("playCount").toInt();

	QDomNode			child	= element.firstChild();
	QDomElement			childElement;
	QDomElement			artistElement;

	while(!child.isNull())
	{
		childElement	= child.toElement();

		if(!childElement.tagName().compare("child"))
		{
			if(childElement.hasAttribute("contentType"))
			{
				TRACK	track;
				qint32	trackID;

				trackID				= childElement.attribute("id").toInt();
				track.parent		= childElement.attribute("parent").toInt();
				track.title			= childElement.attribute("title");
				track.album			= childElement.attribute("album");
				track.artist		= childElement.attribute("artist");
				track.track			= childElement.attribute("track");
				track.year			= childElement.attribute("year").toInt();
				track.genre			= childElement.attribute("genre");
				track.coverArt		= childElement.attribute("coverArt").toInt();
				track.size			= childElement.attribute("size").toInt();
				track.contentType	= childElement.attribute("contentType");
				track.suffix		= childElement.attribute("suffix");
				track.duration		= childElement.attribute("duration").toInt();
				track.bitRate		= childElement.attribute("duration").toInt();
				track.path			= childElement.attribute("path");
				track.isVideo		= childElement.attribute("isVideo") == "true";
				track.playCount		= childElement.attribute("playCount").toInt();
				track.discNumber	= childElement.attribute("discNumber");
				track.created		= QDateTime::fromString(childElement.attribute("created"), "yyyy-MM-ddThh:mm:ss.zzzZ");
				track.albumID		= childElement.attribute("albumID").toInt();
				track.artistID		= childElement.attribute("artistID").toInt();
				track.type			= childElement.attribute("type");

				tracks.insert(trackID, track);
			}
		}

		child	= child.nextSibling();
	}

	return(true);
}

bool cSubsonic::getGenres(QMap<QString, GENRE>& genres)
{
	QDomElement				element;

	if(!sendRequest("getGenres", element, "genres"))
		return(false);

	QDomNode			child	= element.firstChild();
	QDomElement			childElement;

	while(!child.isNull())
	{
		childElement	= child.toElement();

		if(!childElement.tagName().compare("genre"))
		{
			GENRE	genre;
			QString	name;

			genre.songCount		= childElement.attribute("songCount").toInt();
			genre.albumCount	= childElement.attribute("albumCount").toInt();
			name				= childElement.text();

			genres.insert(name, genre);
		}

		child	= child.nextSibling();
	}

	return(true);
}

bool cSubsonic::getArtists(QStringList& indexes, QMap<qint32, ARTIST>& artists, const qint32 id)
{
	QMap<QString, QString>	parameter;
	QDomElement				element;

	if(id != -1)
		parameter.insert("musicFolderId", QString::number(id));

	if(!sendRequest("getArtists", element, "artists", parameter))
		return(false);

	QDomNode			child	= element.firstChild();
	QDomElement			childElement;
	QDomElement			artistElement;

	QString				index;

	while(!child.isNull())
	{
		childElement	= child.toElement();

		if(!childElement.tagName().compare("index"))
		{
			index	= childElement.attribute("name");
			indexes.append(index);

			QDomNode	artistChild	= childElement.firstChild();

			while(!artistChild.isNull())
			{
				artistElement	= artistChild.toElement();

				if(!artistElement.tagName().compare("artist"))
				{
					ARTIST	artist;

					artist.index		= index;
					artist.name			= artistElement.attribute("name");
					artist.coverArt		= artistElement.attribute("coverArt");
					artist.albumCount	= artistElement.attribute("albumCount").toInt();

					artists.insert(artistElement.attribute("id").toInt(), artist);
				}

				artistChild	= artistChild.nextSibling();
			}
		}

		child	= child.nextSibling();
	}

	return(true);
}

QString cSubsonic::randomString()
{
	const	QString	possibleCharacters("abcdef0123456789");
	const	int		randomStringLength = 6;

	QString	randomString;

	for(int i = 0; i < randomStringLength; ++i)
	{
		int		index		= rand() % possibleCharacters.length();
		QChar	nextChar	= possibleCharacters.at(index);
		randomString.append(nextChar);
	}
	return(randomString);
}

bool cSubsonic::sendRequest(const QString& request, QDomElement& element, const QString& verify, const QMap<QString, QString>& parameter)
{
	QNetworkAccessManager	networkManager;
	QString					salt		= randomString();
	QString					token		= QCryptographicHash::hash(QString(m_password+salt).toUtf8(), QCryptographicHash::Md5).toHex();
	QString					strRequest	= QString("%1/rest/%2?u=%3&t=%4&s=%5&v=%6&c=subJuke").arg(m_server, request, m_user, token, salt, m_version);

	if(parameter.count())
	{
		QString				param;

		QMapIterator<QString, QString> i(parameter);
		while(i.hasNext())
		{
			i.next();
			param.append(QString("&%1=%2").arg(i.key(), i.value()));
		}

		strRequest.append(param);
	}

//	qDebug() << strRequest;

	QUrl					url(strRequest);
	QNetworkRequest			networkRequest(url);
	QNetworkReply*			reply   = networkManager.get(networkRequest);
	QEventLoop				loop;

	QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();

	if(reply->error() == QNetworkReply::NoError)
	{
		QDomDocument		doc;
		QString				errorStr;
		int					errorLine;
		int					errorColumn;
		if(!doc.setContent(reply->readAll(), false, &errorStr, &errorLine, &errorColumn))
			return(false);

//		qDebug() << doc.toString();
		element	= doc.documentElement();
		if(element.tagName().compare("subsonic-response"))
			return(false);

		QString				status	= element.attribute("status");
		if(status == "failed")
			return(false);

		if(verify.isEmpty())
		{
			if(element.isNull())
				return(false);
			return(true);
		}

		element	= element.firstChild().toElement();
		if(element.isNull())
			return(false);

		if(element.tagName().compare(verify))
			return(false);
		return(true);
	}
	else
	{
//		qDebug() << reply->errorString();
		return(false);
	}
}
