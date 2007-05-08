/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2007 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#include <kmessagebox.h>
#include <qcursor.h>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>
#include <QKeyEvent>
#include <klocale.h>


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


RoxdokuView::RoxdokuView(ksudoku::Game game, Symbols* symbols, QWidget *parent, const char* /*name*/)
	: QGLWidget(parent)
	, KsView()
	, m_symbols(symbols)
	
{
	m_game = ksudoku::Game();
	m_game = game;
	
	order = m_game.order();
	base = (int) sqrt(order);
	size = base*order;
	connect(m_game.interface(), SIGNAL(cellChange(uint)), this, SLOT(updateGL()));
	connect(m_game.interface(), SIGNAL(completed(bool,const QTime&,bool)), parent, SLOT(onCompleted(bool,const QTime&,bool)));

	wheelmove = 0.0f;
	dist = 5.3f;
	selected_number = -1;

	m_guidedMode = false;

	isClicked  = false;
	isRClicked = false;	
	isDragging = false;	

	selection = -1;
// 	stack_d = 0;
}

RoxdokuView::~RoxdokuView()
{
	glDeleteTextures(10, texture[0]);
	glDeleteTextures(25, texture[1]);
}

QString RoxdokuView::status() const
{
	QString m;

	int secs = QTime(0,0).secsTo(m_game.time());
	if(secs % 36 < 12)
		m = i18n("Selected item %1, Time elapsed %2. DRAG to rotate. MOUSE WHEEL to zoom in/out.",
				 m_symbols->value2Symbol(selected_number, m_game.order()),
		         m_game.time().toString("hh:mm:ss"));
	else  if(secs % 36 < 24)
		m = i18n("Selected item %1, Time elapsed %2. DOUBLE CLICK on a cube to insert selected number.",
				 m_symbols->value2Symbol(selected_number, m_game.order()),
		         m_game.time().toString("hh:mm:ss"));
	else
		m = i18n("Selected item %1, Time elapsed %2. Type in a cell (zero to delete) to place that number in it.",
				 m_symbols->value2Symbol(selected_number, m_game.order()),
		         m_game.time().toString("hh:mm:ss"));

	return m;
}


void RoxdokuView::initializeGL()
{
	glClearColor( 0.0, 0.0, 0.0, 0.5 );
	glEnable(GL_TEXTURE_2D);						// Enable Texture Mapping ( NEW )
	//glShadeModel(GL_SMOOTH);						// Enable Smooth Shading
	//glClearColor(0.0f, 0.0f, 0.0f, 0.5f);					// Black Background
	//glClearDepth(1.0f);							// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);						// Enables Depth Testing
	//glDepthFunc(GL_LEQUAL);							// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	

	setMouseTracking(true);
	
	QPixmap* pixs; 
	for(int o=0; o<2; o++) 
		for(int i=0; i<=9+o*16; i++)
		{
			int sz = 32;
			pixs = new QPixmap(sz,sz);

			QPainter p(pixs);
			QFont f;
			f.setPointSizeF((sz * 80) / 128);
			p.setFont(f);
			p.fillRect(rect(), QColor(255,255,255));
			if(i==0)
				p.drawText(0,0,sz,sz, Qt::AlignCenter, QString(QChar(' ')));
			else{
				QString s = QChar('0'*(o==0) + ('a'-1)*(o==1) +i);
				if(s == "9" || s == "6" || s == "b" || s == "d") s += '.';
				p.drawText(0,0,sz,sz, Qt::AlignCenter, s);
			}
			p.setPen(QPen(QColor(0,0,0), 2));
			p.drawRect ( 0, 0, sz, sz );	
			p.end();
			QImage pix = convertToGLFormat(pixs->toImage());
	
			glGenTextures(1, &texture[o][i]);
			glBindTexture(GL_TEXTURE_2D, texture[o][i]);
			glTexImage2D(GL_TEXTURE_2D, 0,4, sz,sz, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) pix.bits());
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// Linear Filtering
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
			delete pixs;
		}
	/*glEnable(GL_LIGHTING); //UNCOMMENT FOR LIGHTS
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);	
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);
	glEnable(GL_LIGHT1);	
	glEnable(GL_COLOR_MATERIAL);	*/
}

	void RoxdokuView::mouseDoubleClickEvent ( QMouseEvent * /*e*/ )
	{
		if(selection == -1) return;
		if(selected_number == -1) return;
		if(m_game.given(selection)) return;
		m_game.setValue(selection, selected_number);
//		updateGL();
		if(isDragging) releaseMouse();
	}

void RoxdokuView::Selection(int mouse_x, int mouse_y)
{
	if(isDragging)
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

		if(choose <= size && choose > 0)
			selection  = choose-1;

		setFocus();
		paintGL();
	}
	else {
		selection = -1;
	}
}


void RoxdokuView::mouseMoveEvent ( QMouseEvent * e )
{
	Point2fT f;
	f.T[0] = e->x();
	f.T[1] = e->y();
	
	Selection(e->x(), e->y());

	if (isRClicked){                      // If Right Mouse Clicked, Reset All Rotations
		Matrix3fSetIdentity(&LastRot);      // Reset Rotation
		Matrix3fSetIdentity(&ThisRot);      // Reset Rotation
			Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);		// Reset Rotation
	}

	if (!isDragging){          // Not Dragging
		if (isClicked){          // First Click	
		isDragging = true;       // Prepare For Dragging
		LastRot = ThisRot;       // Set Last Static Rotation To Last Dynamic One
		ArcBall->click(&f);      // Update Start Vector And Prepare For Dragging
		grabMouse(/*QCursor(Qt::SizeAllCursor)*/);
		}
		updateGL();
	}
	else{
		if (isClicked){          // Still Clicked, So Still Dragging
			Quat4fT     ThisQuat;

			ArcBall->drag(&f, &ThisQuat);                           // Update End Vector And Get Rotation As Quaternion
			Matrix3fSetRotationFromQuat4f(&ThisRot, &ThisQuat);     // Convert Quaternion Into Matrix3fT
			Matrix3fMulMatrix3f(&ThisRot, &LastRot);                // Accumulate Last Rotation Into This One
			Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);  // Set Our Final Transform's Rotation From This One
		}
		else{                   // No Longer Dragging
			isDragging = false;
			releaseMouse ();
		}
		updateGL();
	}
}

void RoxdokuView::selectValue(int value) {
	selected_number = value;
}

void RoxdokuView::enterValue(int value) {
	if(selection < 0) return;
	
	if(isDragging) releaseMouse();
	m_game.setValue(selection, value);
}


void RoxdokuView::myDrawCube(int name, GLfloat x, GLfloat y, GLfloat z, int /*texturef*/)
{
	glPushMatrix();
	glLoadName(name+1);
	glTranslatef(x,y,z);

	glBindTexture(GL_TEXTURE_2D, texture[order >= 16][m_game.value(name)]);
	
	float sz = 1.0f;
	float s = 0.1f;
	if(selection != -1 && selection != name && m_game.puzzle()->hasConnection(selection, name)) {
		s = -0.25f;
		sz = 0.52f;
		
		switch(m_game.buttonState(name)) {
			case ksudoku::GivenValue:
				glColor3f(0.4f,0.4f,0.8f);
				sz+=0.15;
				break;
			case ksudoku::ObviouslyWrong:
			case ksudoku::WrongValue:
				if(m_guidedMode && game().puzzle()->hasSolution())
					glColor3f(0.75f,0.25f,0.25f);
				else
					glColor3f(0.5f+s,0.5f+s,1.0f+s);
				break;
			case ksudoku::Marker:
			case ksudoku::CorrectValue:
				glColor3f(0.5f+s,0.5f+s,1.0f+s);	
				break;
		}
	} else {
		sz = 1.0f;
		s = 0.1f;
		switch(m_game.buttonState(name)) {
			case ksudoku::GivenValue:
				glColor3f(0.35f,0.70f,0.45f);
				break;
			case ksudoku::ObviouslyWrong:
			case ksudoku::WrongValue:
				if(m_guidedMode && game().puzzle()->hasSolution())
	 				glColor3f(0.75f,0.25f,0.25f);
				else
					glColor3f(0.5f+s,0.5f+s,1.0f+s);
				break;
			case ksudoku::Marker:
			case ksudoku::CorrectValue:
				glColor3f(0.5f+s,0.5f+s,1.0f+s);	
				break;
		}
	}

	if(selection == name)
		glColor3f(0.75f,0.25f,0.25f);

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
	glTranslatef(0.0f, 0.0f, -dist*(base+3)+wheelmove);

	glMultMatrixf(Transform.M);

	int c=0;

	for(int xx=0; xx<base; ++xx)
		for(int yy=0; yy<base; ++yy)
			for(int zz=0; zz<base; ++zz){
				glPushMatrix();
				glTranslatef(-(dist*base-dist)/2,-(dist*base-dist)/2,-(dist*base-dist)/2);
				myDrawCube(c++,(GLfloat) (dist*xx), (GLfloat)(dist* yy ), (GLfloat) (dist*zz), 0);
				glPopMatrix();
			}

	swapBuffers();
}

}

#include "roxdokuview.moc"
