/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KSUDOKUEXPORTDLGSETTINGS_H
#define KSUDOKUEXPORTDLGSETTINGS_H

#include <kconfig.h>


namespace ksudoku {

#define EXPGAMEHCOUNT  "exportGameHCount"
#define EXPGAMEVCOUNT  "exportGameVCount"
#define EXPGAMEMARGIN  "exportGameMargin"
#define EXPGAMEASPECTR "exportGameAspectRatio"
#define EXPGENPREVIEW  "exportGameGeneratePreview"
#define USECURRGAME    "exportUseCurrentGame"

#define EXPGAMEPAGESIZE  "exportGamePageSize"
#define EXPGAMEHRES      "exportGamePageHorRes"
#define EXPGAMEVRES      "exportGamePageVertRes"
#define EXPGAMEPSIZELOCK "exportGamePageSizeLock"

/**
	Settings for Export GUI

	@author 
*/
class ExportDlgSettings{
public:
	ExportDlgSettings();
	~ExportDlgSettings();

	void storeSettings()  ;// { m_kconfig. ; }
	void retrieveSettings();

	//getters
	int   gameHCount() const { return m_kconfig.readEntry( EXPGAMEHCOUNT,  "1").toInt(); }
	int   gameVCount() const { return m_kconfig.readEntry( EXPGAMEVCOUNT,  "1").toInt(); }
	int   gameMargin() const { return m_kconfig.readEntry( EXPGAMEMARGIN, "10").toInt(); }
	float gameAspectRatio()     const { return m_kconfig.readEntry( EXPGAMEASPECTR , "1.0").toFloat(); }
	bool  generatePreviewGame() const { return m_kconfig.readEntry( EXPGENPREVIEW  , "1"  ) != "0"; }
	bool  useCurrentGame     () const { return m_kconfig.readEntry( USECURRGAME    , "1"  ) != "0"; }
	
	///@todo change this to global paper size
	QString pageSize()  const { return m_kconfig.readEntry( EXPGAMEPAGESIZE, "A4"); }
	int     pSizeHRes() const { return m_kconfig.readEntry( EXPGAMEHRES,  "-1").toInt(); }
	int     pSizeVRes() const { return m_kconfig.readEntry( EXPGAMEVRES,  "-1").toInt(); }
	bool    pSizeLock() const { return m_kconfig.readEntry( EXPGAMEPSIZELOCK, "1"  ) != "0"; }

	//setters
	void setGameHCount(int val) { m_kconfig.writeEntry( EXPGAMEHCOUNT , val); }
	void setGameVCount(int val) { m_kconfig.writeEntry( EXPGAMEVCOUNT , val); }
	void setGameMargin(int val) { m_kconfig.writeEntry( EXPGAMEMARGIN , val); }
	void setGameAspectRatio    (float val) { m_kconfig.writeEntry( EXPGAMEASPECTR , val); }
	void setGeneratePreviewGame(bool  val) { m_kconfig.writeEntry( EXPGENPREVIEW  , (val == 0)?0:1); }
	void setUseCurrentGame     (bool  val) { m_kconfig.writeEntry( USECURRGAME    , (val == 0)?0:1); }

	void setPageSize(QString val) { m_kconfig.writeEntry( EXPGAMEPAGESIZE , val); }
	void setPSizeHRes(int val)    { m_kconfig.writeEntry( EXPGAMEHRES     , val); }
	void setPSizeVRes(int val)    { m_kconfig.writeEntry( EXPGAMEVRES     , val); }
	void setPSizeLock(int val)    { m_kconfig.writeEntry( EXPGAMEPSIZELOCK, (val == 0)?0:1); }

private:
	///reference to sessionConfig
	KConfig& m_kconfig;
};

}

#endif
