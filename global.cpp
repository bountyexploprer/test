/*
 * CmdLauncher
 *
 * Copyright (c) 2011 Hong Xu
 *
 *
 * This file is part of CmdLauncher.
 *
 * CmdLauncher is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * CmdLauncher is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with CmdLauncher. If not, see <http://www.gnu.org/licenses/>.
 */

#include "global.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QRegExp>
#include <QSettings>
#include <QStringList>
#include <QTextStream>
#include <cstdlib>

Global::Global()
{
    QStringList arguments = qApp->arguments();

    // set default startup geometry
    startupGeometry.setWidth(800);
    startupGeometry.setHeight(600);
    startupGeometry.setX((QApplication::desktop()->width() -
                          startupGeometry.width())/2);
    startupGeometry.setY((QApplication::desktop()->height() -
                          startupGeometry.height())/2);

    // parse the arguments first
    // we don't need the first argument
    arguments.pop_front();
    bool file_flag = false;
    bool geometry_flag = false;
    Q_FOREACH(const QString& arg, arguments)
    {
        if(file_flag)
        {
            file_flag = false;
            this->iniFile = arg;
        }
        else if(geometry_flag)
        {
            geometry_flag = false;

            // set startup geometry from argument list
            startupGeometry = convertGeometryStringToRect(arg);
        }
        else if(arg == "-f" || arg == "--file")
            file_flag = true;
        else if(arg == "--geometry")
            geometry_flag = true;
        else if(arg == "--help")
        {
            Global::printHelp();
            exit(0);
        }
        else if(this->iniFile.isEmpty())
            this->iniFile = arg;
        else
        {
            QTextStream out(stderr, QIODevice::WriteOnly);

            out << "Arguments error" << endl;

            exit(1);
        }
    }

    // parse the ini file
    QSettings ini(this->iniFile, QSettings::IniFormat);

    // "general" section
    this->command = ini.value("cmd").toString();
    this->tabs = ini.value("tabs").toStringList();
    this->windowTitle = ini.value("title").toString();

    // "items" section
    ini.beginGroup("items");

    Q_FOREACH(const QString& item, ini.childGroups())
    {
        ini.beginGroup(item);

        // read them into a new Global::Item
        Global::Item* new_item = new Global::Item();
        Q_FOREACH(const QString& key, ini.allKeys())
        {
           (*new_item)[key] = ini.value(key);
        }

        this->items.append(new_item);

        ini.endGroup(); // ini.beginGroup(item);
    }

    ini.endGroup(); // ini.beginGroup("items");

    ini.beginGroup("about");

    about.name = ini.value("name").toString();
    about.description = ini.value("description").toString();
    about.authors = ini.value("authors").toStringList();
    about.url = ini.value("url").toString();
    about.pixmapFile = ini.value("pixmap").toString();

    ini.endGroup(); // ini.beginGroup("about");

    // terminal information
    Terminal* tmpterm;
#if defined(Q_WS_X11) || defined(Q_WS_MAC)
    tmpterm = new Terminal();
    tmpterm->name = "xterm";
    tmpterm->cmd = "xterm -hold -e";
    terminals.append(tmpterm);
#endif
#ifdef Q_WS_X11
    tmpterm = new Terminal();
    tmpterm->name = "konsole";
    tmpterm->cmd = "konsole --hold -e";
    terminals.append(tmpterm);
#endif
#ifdef Q_WS_WIN
    tmpterm = new Terminal();
    tmpterm->name = "cmd";
    tmpterm->cmd = "cmd /K";
    terminals.append(tmpterm);
#endif
}

Global* Global::getInstance()
{
    static Global* gi = NULL;

    if(!gi)
        gi = new Global();

    return gi;
}

const QList<Global::Item*>* Global::getItems()
{
    return &this->items;
}

const QString* Global::getCommand()
{
    return &this->command;
}

const QStringList* Global::getTabs()
{
    return &this->tabs;
}

const QList<Global::Terminal*>* Global::getTerminals()
{
    return &this->terminals;
}

const QString* Global::getWindowTitle()
{
    return &this->windowTitle;
}

const Global::About* Global::getAbout()
{
    return &this->about;
}

void Global::setItemTabpageRow(int index, int tab, int row)
{
    Global::Item* item = this->items[index];

    item->insert("tabpage", tab);
    item->insert("row", row);
}

/*
 * print help message
 */
void Global::printHelp()
{
    QTextStream out(stdout, QIODevice::WriteOnly);

    out << QObject::tr(
               "Usage: cmdlauncher [arguments] file\n"
               "\n"
               "arguments:\n"
               "\n"
               "--geometry\t\t\tthe startup geometry of the window."
               " Format is like this: widthxheight+x+y.\n"
               "\t\t\t\tExample: 800x600+50+50\n"
               "--file\tor\t-f\t\tThe cla file specified\n"
               "--help\t\t\t\tPrint this help message\n"
               ) << endl;
}

const QRect* Global::getStartupGeometry()
{
    return &startupGeometry;
}

/*
 * convert geometry string to a QRect
 */
QRect Global::convertGeometryStringToRect(const QString& geostr)
{
    QRect ret;

    QRegExp re("[\\*x\\+]");
    const QStringList l = geostr.split(re);
    int len = l.length();
    if(len > 0)
        ret.setWidth(l.at(0).toInt());
    if(len > 1)
        ret.setHeight(l.at(1).toInt());
    if(len > 2)
        ret.setX(l.at(2).toInt());
    if(len > 3)
        ret.setY(l.at(3).toInt());

    return ret;
}
