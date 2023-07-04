/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2008 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2012      Ian Wadham <iandw.au@gmail.com>                   *
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

#include "roxdokuview.h"

#include "puzzle.h"
#include "ksudoku.h"
#include "skgraph.h"

#include <QCursor>
#include <QPixmap>
#include <KLocalizedString>

#include "settings.h"

#include "renderer.h"
#include "gameactions.h"

namespace ksudoku{

GLUquadricObj *quadratic; // Used For Our Quadric

//const float  = 2.0*3.1415926535f; // PI Squared



GLfloat LightAmbient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightDiffuse[]  = { 0.8f, 1.0f, 1.0f, 1.0f };	
GLfloat LightPosition[] = { 0.0f, 0.0f, -10.0f, 5.0f };		

Matrix4fT Transform   =  {{ {1.0f},  {0.0f},  {0.0f},  {0.0f}, // NEW: Final Transform
                            {0.0f},  {1.0f},  {0.0f},  {0.0f},
                            {0.0f},  {0.0f},  {1.0f},  {0.0f},
                            {0.0f},  {0.0f},  {0.0f},  {1.0f} }};

Matrix3fT LastRot     = {{  {1.0f},  {0.0f},  {0.0f},          // NEW: Last Rotation
                            {0.0f},  {1.0f},  {0.0f},
                            {0.0f},  {0.0f},  {1.0f} }};

Matrix3fT ThisRot     = {{  {1.0f},  {0.0f},  {0.0f},          // NEW: This Rotation
                            {0.0f},  {1.0f},  {0.0f},
                            {0.0f},  {0.0f},  {1.0f} }};


RoxdokuView::RoxdokuView(const ksudoku::Game &game, GameActions * gameActions,
				QWidget * parent)
	: QGLWidget(parent)
{
	m_game   = game;
	m_graph  = m_game.puzzle()->graph();

	m_order  = m_graph->order();
	m_base   = m_graph->base();
	m_size   = m_graph->size();
	m_width  = m_graph->sizeX();
	m_height = m_graph->sizeY();
	m_depth  = m_graph->sizeZ();

	connect(m_game.interface(), &GameIFace::cellChange, this, &QGLWidget::updateGL);
	connect(m_game.interface(), &GameIFace::fullChange, this, &QGLWidget::updateGL);
	connect(gameActions, &GameActions::enterValue, this, &RoxdokuView::enterValue);

	// IDW test. m_wheelmove = 0.0f;
	m_wheelmove = -5.0f; // IDW test. Makes the viewport bigger, can see more.
	m_dist = 5.3f;
	m_selected_number = 1;

	loadSettings();

	m_isClicked  = false;
	m_isRClicked = false;	
	m_isDragging = false;	

	m_selection = -1;
	m_lastSelection = -1;
	m_highlights.fill(0, m_size);
	m_timeDelay = false;
	m_delayTimer = new QTimer(this);
	connect(m_delayTimer, &QTimer::timeout, this, &RoxdokuView::delayOver);
}

RoxdokuView::~RoxdokuView()
{
	glDeleteTextures(10, m_texture[0]);
	glDeleteTextures(25, m_texture[1]);
}

void RoxdokuView::enterValue(int value)
{
	if (m_selection >= 0) {
	    m_game.setValue(m_selection, value);
	    updateGL();
	}
}

QString RoxdokuView::status() const
{
	QString m;

// 	int secs = QTime(0,0).secsTo(m_game.time());
// 	if(secs % 36 < 12)
// 		m = i18n("Selected item %1, Time elapsed %2. DRAG to rotate. MOUSE WHEEL to zoom in/out.",
// 				 m_symbols->value2Symbol(m_selected_number, m_game.order()),
// 		         m_game.time().toString("hh:mm:ss"));
// 	else  if(secs % 36 < 24)
// 		m = i18n("Selected item %1, Time elapsed %2. DOUBLE CLICK on a cube to insert selected number.",
// 				 m_symbols->value2Symbol(m_selected_number, m_game.order()),
// 		         m_game.time().toString("hh:mm:ss"));
// 	else
// 		m = i18n("Selected item %1, Time elapsed %2. Type in a cell (zero to delete) to place that number in it.",
// 				 m_symbols->value2Symbol(m_selected_number, m_game.order()),
// 		         m_game.time().toString("hh:mm:ss"));

	return m;
}


void RoxdokuView::initializeGL()
{
	glClearColor( 0.0, 0.0, 0.0, 0.5 );
	glEnable(GL_TEXTURE_2D);	// Enable Texture Mapping ( NEW )
	//glShadeModel(GL_SMOOTH);	// Enable Smooth Shading
	//glClearColor(0.0f, 0.0f, 0.0f, 0.5f);	// Black Background
	//glClearDepth(1.0f);		// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);	// Enables Depth Testing
	//glDepthFunc(GL_LEQUAL);	// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	

	setMouseTracking(true);
	
	for(int o=0; o<2; o++) 
		for(int i=0; i<=9+o*16; i++)
		{
			int sz = 64;
			QPixmap pic = Renderer::instance()->renderSpecial3D(SpecialCell, sz);
			if(i != 0) {
				pic = Renderer::instance()->renderSymbolOn(pic, i, 0, 9+o*16, SymbolPreset);
			}
			QImage pix = convertToGLFormat(pic.toImage());
	
			glGenTextures(1, &m_texture[o][i]);
			glBindTexture(GL_TEXTURE_2D, m_texture[o][i]);
			glTexImage2D(GL_TEXTURE_2D, 0,4, sz,sz, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) pix.bits());
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// Linear Filtering
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
		}
}

void  RoxdokuView::resizeGL(int w, int h ) {
	if (w == 0) w = 1;	
	if (h == 0) h = 1;
	m_arcBall = new ArcBallT((GLfloat)w, (GLfloat)h);

	glViewport(0, 0, (GLint)w, (GLint)h);
	glMatrixMode(GL_PROJECTION); // Select the Projection Matrix
	glLoadIdentity();            // Reset the Projection Matrix

	gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW); // Select the Modelview Matrix
	glLoadIdentity();
}
	

	void RoxdokuView::mouseDoubleClickEvent ( QMouseEvent * /*e*/ )
	{
		if(m_selection == -1) return;
		if(m_selected_number == -1) return;
		if(m_game.given(m_selection)) return;
		m_game.setValue(m_selection, m_selected_number);
//		updateGL();
		if(m_isDragging) releaseMouse();
	}

void RoxdokuView::Selection(int mouse_x, int mouse_y)
{
	if(m_isDragging)
		return;
	
	makeCurrent();
	
	GLuint	buffer[512];
	GLint	hits;

	GLint	viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);
	glSelectBuffer(512, buffer);
	(void) glRenderMode(GL_SELECT);

	glInitNames();
	glPushName(0);

	glMatrixMode(GL_PROJECTION);     // Selects The Projection Matrix
	glPushMatrix();                  // Push The Projection Matrix
	glLoadIdentity();                // Resets The Matrix

	// This Creates A Matrix That Will Zoom Up To A Small Portion Of The Screen, Where The Mouse Is.
	gluPickMatrix((GLdouble) mouse_x, (GLdouble) (viewport[3]-mouse_y), 1.0f, 1.0f, viewport);
	gluPerspective(45.0f, (GLfloat) (viewport[2]-viewport[0])/(GLfloat) (viewport[3]-viewport[1]), 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	paintGL();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);	
	hits=glRenderMode(GL_RENDER);

	if (hits > 0){
		int	choose = buffer[3];
		int depth = buffer[1];

		for (int loop = 1; loop < hits; loop++){
			// If This Object Is Closer To Us Than The One We Have Selected
			if (buffer[loop*4+1] < GLuint(depth)){
				choose = buffer[loop*4+3];
				depth  = buffer[loop*4+1];
			}
		}

		if(choose <= m_size && choose > 0)
			m_selection  = choose-1;

		// Stop the timer if the selection is on a cube.
		if (m_timeDelay) {
			m_delayTimer->stop();
			m_timeDelay = false;
		}
		setFocus();
		paintGL();
	}
	else if ((! m_timeDelay) && (m_selection != -1)) {
		// Avoid flickering when the pointer passes between cubes.
		m_delayTimer->start(300);
		m_timeDelay = true;
	}
}

void RoxdokuView::delayOver()
{
	// Remove the highlighting, etc. when the pointer rests between cubes.
	m_delayTimer->stop();
	m_timeDelay = false;
	m_selection = -1;
	paintGL();
}

void RoxdokuView::mouseMoveEvent ( QMouseEvent * e )
{
	Point2fT f;
	f.T[0] = e->x();
	f.T[1] = e->y();
	
	Selection(e->x(), e->y());

	if (m_isRClicked){                      // If Right Mouse Clicked, Reset All Rotations
		Matrix3fSetIdentity(&LastRot);      // Reset Rotation
		Matrix3fSetIdentity(&ThisRot);      // Reset Rotation
			Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);		// Reset Rotation
	}

	if (!m_isDragging){          // Not Dragging
		if (m_isClicked){          // First Click	
		m_isDragging = true;       // Prepare For Dragging
		LastRot = ThisRot;       // Set Last Static Rotation To Last Dynamic One
		m_arcBall->click(&f);      // Update Start Vector And Prepare For Dragging
		grabMouse(/*QCursor(Qt::SizeAllCursor)*/);
		}
		updateGL();
	}
	else{
		if (m_isClicked){          // Still Clicked, So Still Dragging
			Quat4fT     ThisQuat;

			m_arcBall->drag(&f, &ThisQuat);                           // Update End Vector And Get Rotation As Quaternion
			Matrix3fSetRotationFromQuat4f(&ThisRot, &ThisQuat);     // Convert Quaternion Into Matrix3fT
			Matrix3fMulMatrix3f(&ThisRot, &LastRot);                // Accumulate Last Rotation Into This One
			Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);  // Set Our Final Transform's Rotation From This One
		}
		else{                   // No Longer Dragging
			m_isDragging = false;
			releaseMouse ();
		}
		updateGL();
	}
}

void RoxdokuView::selectValue(int value) {
	m_selected_number = value;
}

void RoxdokuView::loadSettings() {
	m_guidedMode        = Settings::showErrors();
	m_showHighlights    = Settings::showHighlights3D();

	float s             = Settings::overallSize3D()/10.0f;	// Normal size.
	m_unhighlightedSize = s;
	m_selectionSize     = s * Settings::selectionSize3D()/10.0f;
	m_highlightedSize   = s * Settings::highlightedSize3D()/10.0f;
	m_outerCellSize     = s * Settings::outerCellSize3D()/10.0f;
	m_darkenOuterCells  = Settings::darkenOuterCells3D();
}

void RoxdokuView::settingsChanged() {
	loadSettings();
	updateGL();
}

void RoxdokuView::myDrawCube(bool highlight, int name,
				GLfloat x, GLfloat y, GLfloat z, bool outside)
{
	glPushMatrix();
	glLoadName(name+1);
	glTranslatef(x,y,z);

	glBindTexture(GL_TEXTURE_2D, m_texture[m_order >= 16][m_game.value(name)]);
	
	float sz = 1.0f;
	float s = 0.2f;
	if(m_selection != -1 && m_selection != name && highlight) {
		s = +0.2;
		sz = m_highlightedSize;

		switch(m_game.buttonState(name)) {
			case ksudoku::GivenValue:
				glColor3f(0.85f,1.0f,0.4f);	// Green/Gold.
				break;
			case ksudoku::ObviouslyWrong:
			case ksudoku::WrongValue:
				if(m_guidedMode && m_game.puzzle()->hasSolution())
					glColor3f(0.75f,0.25f,0.25f);	// Red.
				else
					glColor3f(0.75f+s,0.75f+s,0.25f+s);
				break;
			case ksudoku::Marker:
			case ksudoku::CorrectValue:
				glColor3f(0.75f+s,0.75f+s,0.25f+s);	// Gold.
				break;
		}
	} else {
		sz = m_unhighlightedSize;
		s = 0.1f;
		if (outside && (m_selection != -1)) {
		    // Shrink and darken cells outside the selection-volume.
		    sz = m_outerCellSize;
		    s  = m_darkenOuterCells ? -0.24f : 0.0f;
		}
		switch(m_game.buttonState(name)) {
			case ksudoku::GivenValue:
				glColor3f(0.6f+s,0.9f+s,0.6f+s);	// Green.
				break;
			case ksudoku::ObviouslyWrong:
			case ksudoku::WrongValue:
				if(m_guidedMode && m_game.puzzle()->hasSolution())
	 				glColor3f(0.75f,0.25f,0.25f);	// Red.
				else
					glColor3f(0.6f+s,1.0f+s,1.0f+s);// Blue.
				break;
			case ksudoku::Marker:
			case ksudoku::CorrectValue:
				glColor3f(0.6f+s,1.0f+s,1.0f+s);	// Blue.
				break;
		}
	}

	if(m_selection == name) {
		sz = m_selectionSize;
		// IDW test. glColor3f(0.75f,0.25f,0.25f);
		glColor3f(1.0f,0.8f,0.4f);	// Orange.
	}

	glBegin(GL_QUADS);
	/* front face */
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-sz, -sz, sz); 
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(sz, -sz, sz);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(sz, sz, sz);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-sz, sz, sz);
		/* back face */
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-sz, -sz, -sz); 
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-sz, sz, -sz);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(sz, sz, -sz);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(sz, -sz, -sz);
		/* right face */
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(sz, -sz, -sz); 
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(sz, sz, -sz);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(sz, sz, sz);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(sz, -sz, sz);
		/* left face */
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-sz, -sz, sz); 
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-sz, sz, sz);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-sz, sz, -sz);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-sz, -sz, -sz);
		/* top face */
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(sz, sz, sz); 
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(sz, sz, -sz);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-sz, sz, -sz);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-sz, sz, sz);
		/* bottom face */
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(sz, -sz, -sz); 
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(sz, -sz, sz);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-sz, -sz, sz);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-sz, -sz, -sz);
	glEnd();
	glPopMatrix();
}

void RoxdokuView::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);
	glTranslatef(0.0f, 0.0f, -m_dist*(m_width+3)+m_wheelmove);

	glMultMatrixf(Transform.M);

	enum {Outside, Inside, Highlight};
	int selX = -1, selY = -1, selZ = -1;

	// If a cell is newly selected work out the highlights and lowlights.
	if ((m_selection != -1) && (m_selection != m_lastSelection)) {
	    m_lastSelection = m_selection;
	    selX = m_graph->cellPosX (m_selection);
	    selY = m_graph->cellPosY (m_selection);
	    selZ = m_graph->cellPosZ (m_selection);

	    // Note: m_highlights persists through many frame-paints per second.
	    m_highlights.fill(Outside, m_size);

	    // Mark the cells to be highlighted when highlighting is on.
	    QList<int> groupsToHighlight = m_graph->cliqueList(m_selection);
	    for(int g = 0; g < groupsToHighlight.count(); g++) {
		QVector<int> cellList =
				m_graph->clique(groupsToHighlight.at(g));
		for (int n = 0; n < m_order; n++) {
		    m_highlights[cellList.at(n)] = Highlight;
		}
	    }

	    // Mark non-highlighted cells that are inside cubes containing
	    // the selected cell.  In custom Roxdoku puzzles with > 1 cube,
	    // cells outside are shrunk and darkened.
	    for (int n = 0; n < m_graph->structureCount(); n++) {
		int cubePos = m_graph->structurePosition(n);
		int cubeX   = m_graph->cellPosX(cubePos);
		int cubeY   = m_graph->cellPosY(cubePos);
		int cubeZ   = m_graph->cellPosZ(cubePos);
		if (m_graph->structureType(n) != SKGraph::RoxdokuGroups) {
		    continue;
		}
		if ((selX >= cubeX) && (selX < (cubeX + m_base)) &&
		    (selY >= cubeY) && (selY < (cubeY + m_base)) &&
		    (selZ >= cubeZ) && (selZ < (cubeZ + m_base))) {
		    for (int x = cubeX; x < cubeX + m_base; x++) {
			for (int y = cubeY; y < cubeY + m_base; y++) {
			    for (int z = cubeZ; z < cubeZ + m_base; z++) {
				int pos = m_graph->cellIndex(x, y, z);
				if (m_highlights.at(pos) == Outside) {
				    m_highlights[pos] = Inside;
				}
			    }
			}
		    }
		}
	    }
	}

	int c = 0;

	for(int xx = 0; xx < m_width; ++xx) {
		for(int yy = 0; yy < m_height; ++yy) {
			for(int zz = 0; zz < m_depth; ++zz) {
				if(m_game.value(c) == UNUSABLE) {
				    c++;
				    continue;	// Do not paint unusable cells.
				}
				glPushMatrix();

				// Centre the puzzle in the viewport.
				glTranslatef(-(m_dist * m_width  - m_dist) / 2,
					     -(m_dist * m_height - m_dist) / 2,
					     -(m_dist * m_depth  - m_dist) / 2);

				// Highlight cells in the three planes through
				// the selected cell. Unhighlight cells outside
				// the cubical volume of the selection.
				bool highlight = m_showHighlights &&
					     (m_highlights.at(c) == Highlight);
				bool outside = (m_highlights.at(c) == Outside);

				myDrawCube(highlight, c++,
					    (GLfloat)(m_dist * xx),
					    (GLfloat)(m_dist * yy),
					    (GLfloat)(m_dist * zz), outside);

				glPopMatrix();
			}
		}
	}
	swapBuffers();
}

}

#include "moc_roxdokuview.cpp"
