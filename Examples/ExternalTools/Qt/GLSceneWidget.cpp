/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE 			                                                         */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : GLSceneWidget.cpp                                                                         */
/*     Original Date : May 26th 2014                                                                             */
/*                                                                                                               */
/*     Description   : Qt interface for visualization with OpenGL.                                               */
/*                                                                                                               */
/* ************************************************************************************************************* */

#include "GLSceneWidget.hpp"

using namespace QVGL;

// View :
	View::View(HdlTexture* _texture, const QString& _name)
	 : 	texture(_texture),
		name(_name),
		qvglParent(NULL),
		angle(0.0f),
		homothecyScale(1.0f),
		flipUpDown(false),
		flipLeftRight(false)
	{
		reset();

		if(texture==NULL)
			throw Exception("View::View - Texture pointer is NULL.", __FILE__, __LINE__);
	}

	View::~View(void)
	{
		emit closed();
	}

	void View::prepareToDraw(void)
	{
		texture->bind();
	}

	const __ReadOnly_HdlTextureFormat& View::getFormat(void) const
	{
		return *texture; // TODO : fix the return temporary warning.
	}

	const QString& View::getName(void) const
	{
		return name;
	}

	void View::setName(const QString& newName)
	{
		name = newName;

		emit nameChanged();
	}

	float View::getAngle(void) const
	{
		return angle;
	}

	void View::setAngle(const float& a)
	{
		angle = a;
		
		emit updated();
	}

	void View::rotate(const float& a)
	{
		angle += a;

		emit updated();
	}

	void View::getViewCenter(float& x, float& y) const
	{
		x = viewCenter[0];
		y = viewCenter[1];
	}

	void View::setViewCenter(const float& x, const float& y)
	{
		viewCenter[0] = x;
		viewCenter[1] = y;

		emit updated();
	}

	void View::move(const float& x, const float& y)
	{
		viewCenter[0] += x;
		viewCenter[1] += y;

		emit updated();
	}

	bool View::isMirrored(void) const
	{
		return flipLeftRight;
	}

	void View::setMirror(bool enabled)
	{
		flipLeftRight = enabled;

		emit updated();
	}

	bool View::isUpsideDown(void) const
	{
		return flipUpDown;	
	}

	void View::setUpsideDown(bool enabled)
	{
		flipUpDown = enabled;

		emit updated();
	}

	void View::zoom(const float& xCenter, const float& yCenter, const float& factor)
	{
		// factor > 1.0 => zoom in
		float 	a		= homothecyScale * factor - factor,
			b		= factor - 1.0f,
			c		= homothecyScale * factor - 1.0f;

		if(std::abs(c) <= 100.0f * std::numeric_limits<float>::epsilon()) // This composition is a translation
		{
			homothecyCenter[0]	= (1.0f - factor) * (xCenter - homothecyCenter[0]);
			homothecyCenter[1]	= (1.0f - factor) * (yCenter - homothecyCenter[1]);
			homothecyScale		= 0.0f;
		}
		else if( homothecyScale==0.0f ) // From a translation to a homothecy
		{
			homothecyCenter[0]	= (xCenter * (1.0f - factor) + factor * homothecyCenter[0]) / (1.0f - factor);
			homothecyCenter[1]	= (yCenter * (1.0f - factor) + factor * homothecyCenter[1]) / (1.0f - factor);
			homothecyScale		= factor;
		}
		else // CGeneric case
		{
			homothecyCenter[0]	= (a * homothecyCenter[0] + b * xCenter)/c;
			homothecyCenter[1]	= (a * homothecyCenter[1] + b * yCenter)/c;
			homothecyScale	 	= homothecyScale * factor;
		}

		emit updated();
	}

	void View::reset(void)
	{
		angle			= 0.0f;
		homothecyScale		= 1.0f;
		flipUpDown		= false;
		flipLeftRight		= false;
		viewCenter[0]		= 0.0f;
		viewCenter[1]	 	= 0.0f;
		homothecyCenter[0]	= 0.0f;
		homothecyCenter[1]	= 0.0f;

		emit updated();
	}

	void View::show(void)
	{
		emit requireDisplay();
	}

	void View::close(void)
	{
		emit closed();
	}

	bool View::isClosed(void)
	{
		return qvglParent==NULL;
	}

	float View::getImageRatio(void) const
	{
		if(texture==NULL)
			return 0.0f;
		else
			return static_cast<float>(texture->getWidth()) / static_cast<float>(texture->getHeight());
	}
	
	void View::getAspectRatioScaling(float& xImgScale, float& yImgScale) const
	{
		const float imgRatio = getImageRatio();

		xImgScale = std::min(1.0f, imgRatio);
		yImgScale = std::min(1.0f, 1.0f/imgRatio);

		if(flipLeftRight)	xImgScale *= -1.0f;
		if(flipUpDown) 		yImgScale *= -1.0f;
	}

	QString View::getSizeString(void) const
	{
		if(texture!=NULL)
		{
			const unsigned int sizeInBytes = texture->getSize();

			const float	GB = 1024.0f*1024.0f*1024.0f,
					MB = 1024.0f*1024.0f,
					KB = 1024.0f;

			if(sizeInBytes>=GB)
				return tr("%1 GB").arg( static_cast<unsigned int>(std::ceil(static_cast<float>(sizeInBytes)/GB)) );
			else if(sizeInBytes>=MB)
				return tr("%1 MB").arg( static_cast<unsigned int>(std::ceil(static_cast<float>(sizeInBytes)/MB)) );
			else if(sizeInBytes>=KB)
				return tr("%1 KB").arg( static_cast<unsigned int>(std::ceil(static_cast<float>(sizeInBytes)/KB)) );
			else 
				return tr("%1 B").arg( sizeInBytes );
		}
		else
			return "0 B";
	}

// SubWidget :
	SubWidget::SubWidget(void)
	 : 	layout(this),
		titleBar(&titleWidget),
		titleLabel(this),
		hideButton(this),
		widget(NULL),
		visible(false),
		motionActive(false),
		resizeActive(false),
		resizeHorizontalLock(false),
		resizeVerticalLock(false),
		qvglParent(NULL),
		graphicsProxy(NULL)
	{
		// But first : 
		QWidget::hide();

		// Activate focus via mouse click :
		setFocusPolicy(Qt::ClickFocus);

		// Create title bar : 
		titleLabel.setOpenExternalLinks(false);
		setTitle("Title");

		// Buttons : 
		const int 	w = 32,
				h = 16;
		hideButton.setMinimumSize(w, h);
		hideButton.setMaximumSize(w, h);

		titleBar.addWidget(&titleLabel);
		titleBar.addWidget(&hideButton);
		titleBar.setMargin(0);
		titleBar.setSpacing(0);

		titleWidget.setFixedHeight(h);

		// Create final layout : 
		layout.addWidget(&titleWidget, 0);
		//layout.addWidget(widget, 1);
		layout.setMargin(4);
		layout.setSpacing(1);

		// Connections : 
		QObject::connect(&hideButton, SIGNAL(released(void)), this, SLOT(hide(void)));
	}

	SubWidget::~SubWidget(void)
	{ }

	void SubWidget::mousePressEvent(QMouseEvent* event)
	{
		QWidget::mousePressEvent(event);

		if(!event->isAccepted() && qvglParent!=NULL)
		{
			event->accept(); // Do not propagate the event hereafter
			
			offset = event->pos();

			const int borderResize = 4;
			//resizeActive = (std::abs(event->pos().x())<borderResize || std::abs(event->pos().x()-width())<borderResize || std::abs(event->pos().y())<borderResize || std::abs(event->pos().y()-height())<borderResize); // All borders
			resizeActive = (std::abs(event->pos().x()-width())<borderResize || std::abs(event->pos().y()-height())<borderResize);
			motionActive = !resizeActive;

			if(resizeActive)
			{
				resizeHorizontalLock 	= (std::abs(event->pos().y()-height())>borderResize);
				resizeVerticalLock	= (std::abs(event->pos().x()-width())>borderResize);
			}
		}

		// Send selection signal : 
		emit selected(this);
	}

	void SubWidget::mouseMoveEvent(QMouseEvent* event)
	{
		QWidget::mousePressEvent(event);

		if(!event->isAccepted() && qvglParent!=NULL)
		{
			event->accept(); // Do not propagate the event hereafter.

			if(motionActive)
			{
				// Get the new position : 
				QPoint v = mapToParent(event->pos() - offset);

				// Prevent the widget from moving out of the scene : 
				v.setX( std::min(static_cast<int>(qvglParent->sceneRect().width()-width()), v.x()) );
				v.setY( std::min(static_cast<int>(qvglParent->sceneRect().height()-titleBar.geometry().height() - 4), v.y()) );

				v.setX( std::max(0, v.x()) );
				v.setY( std::max(0, v.y()) );
			
				// Move : 
				move(v);
			}
			else if(resizeActive)
			{
				QPoint d = event->pos() - offset;

				if(resizeHorizontalLock)
					d.setY(0);

				if(resizeVerticalLock)
					d.setX(0);

				d.setX( std::min(static_cast<int>(qvglParent->sceneRect().width()-x()-width()), d.x()) );
				d.setY( std::min(static_cast<int>(qvglParent->sceneRect().height()-y()-height()), d.y()) );

				resize(width()+d.x(), height()+d.y());

				// Re-offset : 
				offset = event->pos();
			}
		}

		//std::cout << "GLSubWidget : Mouse move event" << std::endl;
	}

	void SubWidget::mouseReleaseEvent(QMouseEvent* event)
	{
		QWidget::mousePressEvent(event);

		if(!event->isAccepted())
		{
			event->accept(); // Do not propagate the event hereafter.
			
			motionActive = false;
			resizeActive = false;
		}
	}

	void SubWidget::temporaryHide(bool enabled)
	{
		if(enabled)
			QWidget::hide();
		else if(!enabled && visible)
			QWidget::show();
	}

	void SubWidget::setInnerWidget(QWidget* _widget)
	{
		if(widget!=NULL)
			throw Exception("SubWidget::setInnerWidget - A QWidget was already bound.", __FILE__, __LINE__);
		else if(_widget==NULL)
			throw Exception("SubWidget::setInnerWidget - Invalid QWidget (NULL).", __FILE__, __LINE__);
		else
		{
			widget = _widget;
			layout.addWidget(widget, 1);
		}
	}

	QWidget* SubWidget::innerWidget(void)
	{
		return widget;
	}

	// Deprecated : 
	/*void SubWidget::setWidget(QWidget* widget)
	{
		layout.addWidget(widget, 1);
	}

	void SubWidget::setLayout(QLayout* subLayout)
	{
		//if(layout.itemAt(2)!=NULL)
		//{
		//	std::cout << "Remove!" << std::endl;
		//	layout.takeAt(2);
		//}
		
		layout.addLayout(subLayout, 1);
	}*/

	QString SubWidget::getTitle(void)
	{
		return titleLabel.text();
	}

	void SubWidget::setTitle(QString title)
	{
		// Do not put rich text!
		title.remove(QRegExp("<[^>]*>"));

		titleLabel.setText(title);
		titleLabel.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		emit titleChanged();
	}

	MainWidget* SubWidget::getQVGLParent(void)
	{
		return qvglParent;
	}

	bool SubWidget::shoudBeVisible(void) const
	{
		return visible;
	}

	void SubWidget::show(void)
	{
		QWidget::show();

		visible = true;

		emit showRequest(this);
	}

	void SubWidget::hide(void)
	{
		QWidget::hide();

		visible = false;

		emit hideRequest(this);
	}

// PositionColorInfoMini : 
	PositionColorInfoMini::PositionColorInfoMini(void)
	 : bar(this)
	{
		colorBox.setAutoFillBackground(true);
		QFontMetrics fontMetrics(positionLabel.font());

		positionLabel.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		positionLabel.setFixedWidth(fontMetrics.width("00000x00000"));
		
		colorBox.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		colorBox.setFixedWidth(fontMetrics.width("####"));

		bar.addWidget(&positionLabel);
		bar.addWidget(&colorBox);
		bar.addSpacing(2);

		bar.setMargin(0);
		bar.setSpacing(0);

		setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

		// Clear : 
		updatePosition();
		updateColor();
	}

	PositionColorInfoMini::~PositionColorInfoMini(void)
	{ }

	void PositionColorInfoMini::setWindowOpacity(qreal level)
	{
		QWidget::setWindowOpacity(level);
		colorBox.setWindowOpacity(0.0);		// TODO : to fix missing solid color.
	}

	void PositionColorInfoMini::updatePosition(const QPointF& pos)
	{
		//positionLabel.setText("00000x00000");
		positionLabel.setText(tr("%1x%2").arg(std::floor(pos.x())).arg(std::floor(pos.y())));
	}

	void PositionColorInfoMini::updateColor(const QColor& color)
	{
		QPalette palette = colorBox.palette();
		palette.setColor(QPalette::Window, color);
		colorBox.setPalette(palette);
	}

// TopBar
	TopBar::TopBar(void)
	 : 	graphicsProxy(NULL),
		bar(this),
		mainMenuButton("Menu"),
		viewsMenuButton("Views"),
		widgetsMenuButton("Widgets"),
		mainMenu("Menu", this),
		viewsMenu("Views", this),
		widgetsMenu("Widgets", this),
		temporaryHideAllSubWidgetsAction("Hide all widgets", this),
		hideAllSubWidgetsAction("Close all widgets", this)
	{
		// Activate focus via mouse click :
		setFocusPolicy(Qt::ClickFocus);

		// Menus : 
		mainMenuButton.setMenu(&mainMenu);
		viewsMenuButton.setMenu(&viewsMenu);
		widgetsMenuButton.setMenu(&widgetsMenu);

		mainMenuButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		viewsMenuButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		widgetsMenuButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

		bar.addWidget(&mainMenuButton);
		bar.addWidget(&viewsMenuButton);
		bar.addWidget(&widgetsMenuButton);
		bar.addWidget(&titleLabel);
		bar.addWidget(&positionColorInfo);

		bar.setMargin(0);
		bar.setSpacing(0);

		// Signals connections : 
		QObject::connect(&viewsSignalMapper, 			SIGNAL(mapped(QObject*)), 	this, SLOT(castViewPointer(QObject*)));
		QObject::connect(&widgetsSignalMapper,			SIGNAL(mapped(QObject*)), 	this, SLOT(castSubWidgetPointer(QObject*)));
		QObject::connect(&mainMenu,				SIGNAL(aboutToShow()),		this, SLOT(sendSelectedSignal()));
		QObject::connect(&viewsMenu,				SIGNAL(aboutToShow()),		this, SLOT(sendSelectedSignal()));
		QObject::connect(&widgetsMenu,				SIGNAL(aboutToShow()),		this, SLOT(sendSelectedSignal()));
		QObject::connect(&temporaryHideAllSubWidgetsAction,	SIGNAL(triggered()),		this, SIGNAL(temporaryHideAllSubWidgets()));
		QObject::connect(&hideAllSubWidgetsAction,		SIGNAL(triggered()),		this, SIGNAL(hideAllSubWidgets()));

		// Reset : 
		setTitle();
		updateViewsList(QList<View*>());
		updateSubWidgetsList(QList<SubWidget*>());

		// TEST :
		mainMenu.addAction("Action 1");
		mainMenu.addAction("Action 2");
		mainMenu.addAction("Action 3");
	}

	TopBar::~TopBar(void)
	{ }

	void TopBar::mousePressEvent(QMouseEvent* event)
	{
		QWidget::mousePressEvent(event);

		emit selected(this);
	}

	void TopBar::stretch(const QRectF& rect)
	{
		setGeometry(0, 0, rect.width(), height());
	}

	void TopBar::castViewPointer(QObject* ptr)
	{
		emit changeViewRequest(reinterpret_cast<View*>(ptr));
	}

	void TopBar::castSubWidgetPointer(QObject* ptr)
	{
		emit showSubWidgetRequest(reinterpret_cast<SubWidget*>(ptr));
	}

	void TopBar::sendSelectedSignal(void)
	{
		emit selected(this);
	}

	void TopBar::setTitle(void)
	{
		titleLabel.setText("(No Title)");
		titleLabel.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		// No tooltip : 
		titleLabel.setToolTip("");
	}

	void TopBar::setTitle(QString title)
	{
		// Do not put rich text!
		title.remove(QRegExp("<[^>]*>"));

		titleLabel.setText(title);
		titleLabel.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		// No tooltip : 
		titleLabel.setToolTip("");
	}

	void TopBar::setTitle(const View& view)
	{
		// Set title : 
		titleLabel.setText(view.getName());
		titleLabel.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		// Set tooltip info :
		QString toolTip;

		toolTip += "<table>";
			toolTip += tr("<tr><td><i>Size</i></td><td>:</td><td>%1x%2 (%3)</td></tr>").arg(view.getFormat().getWidth()).arg(view.getFormat().getHeight()).arg(view.getSizeString());
			toolTip += tr("<tr><td><i>Mode</i></td><td>:</td><td>%1</td></tr>").arg(glParamName( view.getFormat().getGLMode() ).c_str());
			toolTip += tr("<tr><td><i>Depth</i></td><td>:</td><td>%1</td></tr>").arg(glParamName( view.getFormat().getGLDepth() ).c_str());
			toolTip += tr("<tr><td><i>Filtering</i></td><td>:</td><td>%1 / %2</td></tr>").arg(glParamName( view.getFormat().getMinFilter() ).c_str()).arg(glParamName( view.getFormat().getMagFilter() ).c_str());
			toolTip += tr("<tr><td><i>Wrapping</i></td><td>:</td><td>%1 / %2</td></tr>").arg(glParamName( view.getFormat().getSWrapping() ).c_str()).arg(glParamName( view.getFormat().getTWrapping() ).c_str());
			toolTip += tr("<tr><td><i>Mipmap</i></td><td>:</td><td>%1 / %2</td></tr>").arg(view.getFormat().getBaseLevel()).arg(view.getFormat().getMaxLevel());
		toolTip += "</table>"; 

		titleLabel.setToolTip(toolTip);
	}

	void TopBar::updateViewsList(const QList<View*>& viewsList)
	{
		// Clear previous list : 
		viewsMenu.clear();

		if(!viewsList.isEmpty())
		{
			for(QList<View*>::const_iterator it=viewsList.begin(); it!=viewsList.end(); it++)
			{
				// Create the action and set the mapping : 	
				QAction* tmpAction = viewsMenu.addAction((*it)->getName(), &viewsSignalMapper, SLOT(map()));
				viewsSignalMapper.setMapping(tmpAction, reinterpret_cast<QObject*>(*it));
			}
		}
		else
			viewsMenu.addAction("(No Views)");
	}

	void TopBar::updateSubWidgetsList(const QList<SubWidget*>& subWidgetsList)
	{
		// Clear the previous list : 
		widgetsMenu.clear();

		if(!subWidgetsList.isEmpty())
		{
			// Add the default action : 
			widgetsMenu.addAction(&temporaryHideAllSubWidgetsAction);
			widgetsMenu.addAction(&hideAllSubWidgetsAction);
			temporaryHideAllSubWidgetsAction.setEnabled(true);
			hideAllSubWidgetsAction.setEnabled(true);

			widgetsMenu.addSeparator();

			for(QList<SubWidget*>::const_iterator it=subWidgetsList.begin(); it!=subWidgetsList.end(); it++)
			{
				// Create the action and set the mapping : 	
				QAction* tmpAction = widgetsMenu.addAction((*it)->getTitle(), &widgetsSignalMapper, SLOT(map()));
				widgetsSignalMapper.setMapping(tmpAction, reinterpret_cast<QObject*>(*it));
			}
		}
		else
		{
			// Add the default action : 
			widgetsMenu.addAction(&temporaryHideAllSubWidgetsAction);
			widgetsMenu.addAction(&hideAllSubWidgetsAction);
			temporaryHideAllSubWidgetsAction.setEnabled(false);
			hideAllSubWidgetsAction.setEnabled(false);
		}
	}

	void TopBar::updatePositionAndColor(const QPointF& pos, const QColor& color)
	{
		positionColorInfo.updatePosition(pos);
		positionColorInfo.updateColor(color);
	}

	void TopBar::setWindowOpacity(qreal level)
	{
		QWidget::setWindowOpacity(level);
		mainMenu.setWindowOpacity(level);
		viewsMenu.setWindowOpacity(level);
		widgetsMenu.setWindowOpacity(level);
		positionColorInfo.setWindowOpacity(level);
	}

// BottomBar : 
	BottomBar::BottomBar(void)
	 : 	graphicsProxy(NULL),
		bar(this)
	{
		// Activate focus via mouse click :
		setFocusPolicy(Qt::ClickFocus);

		// Bar : 
		bar.addWidget(&toolBar);

		bar.setMargin(0);
		bar.setSpacing(0);

		// Hide : 
		hide();
	}

	BottomBar::~BottomBar(void)
	{ }

	void BottomBar::mousePressEvent(QMouseEvent* event)
	{
		QWidget::mousePressEvent(event);

		emit selected(this);
	}

	void BottomBar::stretch(const QRectF& rect)
	{
		setGeometry(0, rect.height()-height(), rect.width(), height());
	}

// ContextWidget :
	ContextWidget::ContextWidget(QGLContext* ctx, QWidget* parent)
	 : 	QGLWidget(ctx, parent),
		glipOwnership(false),
		clearColorRed(0.1f),
		clearColorGreen(0.1f),
		clearColorBlue(0.1f)
	{
		try
		{
			makeCurrent();

			// Glip-lib specifics : 
			if(!HandleOpenGL::isInitialized())
			{
				glipOwnership = true;
				HandleOpenGL::init();
			}
		}
		catch(Exception& e)
		{
			// Show Error : 
			std::cerr << "GLSceneWidget::GLSceneWidget - Exception caught : " << std::endl;
			std::cerr << e.what() << std::endl;

			QMessageBox errorBox(QMessageBox::Critical, "ContextWidget::ContextWidget", "An error occurred during initialization.", QMessageBox::Ok);
			errorBox.setDetailedText(e.what());
			errorBox.exec();

			// re-throw :
			throw e;
		}
		
		/*setAttribute(Qt::WA_PaintOnScreen);
		setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_OpaquePaintEvent);*/
	}

	ContextWidget::~ContextWidget(void)
	{
		if(glipOwnership)
			HandleOpenGL::deinit();
	}

	void ContextWidget::initializeGL(void)
	{
		makeCurrent();
		glViewport(0, 0, width(), height());
		setAutoBufferSwap(false);
	}

	void ContextWidget::resizeGL(int width, int height)
	{
		glViewport(0, 0, width, height);
	}

// KeyboardState :
	KeyboardState::KeyboardState(void)
	{
		for(int k=0; k<NumActions; k++)
		{
			actionPressed[k] = false;
			takeBackEnabled[k] = false;
		}

		resetActionsKeySequences();
	}

	KeyboardState::~KeyboardState(void)
	{ }

	void KeyboardState::keyPressed(QKeyEvent* event)
	{
		ActionID a = getActionAssociatedToKey(event);

		if(a!=NoAction)
		{
			event->accept();
			actionPressed[a] = true;

			emit actionReceived(a);
		}
	}

	void KeyboardState::keyReleased(QKeyEvent* event)
	{
		ActionID a = getActionAssociatedToKey(event);

		if(a!=NoAction)
		{
			event->accept();
			actionPressed[a] = false;

			if(isActionTakeBackEnabled(a))
				emit actionReceived(a, true);
		}
	}

	ActionID KeyboardState::getActionAssociatedToKey(const QKeySequence& keySequence) const
	{
		if(keysActionsAssociations.contains(keySequence))
			return keysActionsAssociations[keySequence];
		else
			return NoAction;
	}

	ActionID KeyboardState::getActionAssociatedToKey(const QKeyEvent* event) const
	{
		const int s = (event->key() | event->modifiers());

		return getActionAssociatedToKey(QKeySequence(s));
	}

	QKeySequence KeyboardState::getKeysAssociatedToAction(const ActionID& a)
	{
		bool addComa = false;
		QString keysString;

		for(QMap<QKeySequence, ActionID>::iterator it=keysActionsAssociations.begin(); it!=keysActionsAssociations.end(); it++)
		{
			if(it.value()==a)
			{
				if(addComa)
					keysString += ", ";

				keysString += it.key().toString();
				addComa = true;
			}
		}
	
		return QKeySequence(keysString);
	}

	bool KeyboardState::isActionTakeBackEnabled(const ActionID& a) const
	{
		if(a>=0 && a<NumActions)
			return takeBackEnabled[a];
		else
			throw Exception("KeyboardState::isActionTakenBack - Invalid ActionID.", __FILE__, __LINE__);
	}

	void KeyboardState::setTakeBack(const ActionID& a, bool enabled)
	{
		if(a>=0 && a<NumActions)
			takeBackEnabled[a] = enabled;
		else
			throw Exception("KeyboardState::isActionTakenBack - Invalid ActionID.", __FILE__, __LINE__);
	}

	void KeyboardState::setActionKeySequence(const ActionID& a, const QKeySequence& keySequence, bool enableTakeBack)
	{
		// Remove all the previous sequences associated with the action : 
		QList<QKeySequence> previousKeySequence = keysActionsAssociations.keys(a);

		for(QList<QKeySequence>::iterator it=previousKeySequence.begin(); it!=previousKeySequence.end(); it++)
			keysActionsAssociations.remove(*it);

		// Insert the sequences separately : 
		for(int k=0; k<keySequence.count(); k++)
		{
			QKeySequence ks(keySequence[k]);

			// Remove all the sequences interfering with other actions :
			keysActionsAssociations.remove(ks);

			// Insert the new association : 
			keysActionsAssociations[ks] = a;
		}

		// Take back : 
		setTakeBack(a, enableTakeBack);
	}

	void KeyboardState::resetActionsKeySequences(void)
	{
		setActionKeySequence(ActionUp,				Qt::Key_Up);
		setActionKeySequence(ActionDown,				Qt::Key_Down);
		setActionKeySequence(ActionLeft,				Qt::Key_Left);
		setActionKeySequence(ActionRight,				Qt::Key_Right);
		setActionKeySequence(ActionZoomIn,				QKeySequence(Qt::Key_Plus, Qt::SHIFT + Qt::Key_Plus, Qt::KeypadModifier + Qt::Key_Plus));	// Support for keypad and shifts.
		setActionKeySequence(ActionZoomOut,				QKeySequence(Qt::Key_Minus, Qt::SHIFT + Qt::Key_Minus, Qt::KeypadModifier + Qt::Key_Minus));	// Support for keypad and shifts.
		setActionKeySequence(ActionRotationClockWise,		Qt::Key_F);
		setActionKeySequence(ActionRotationCounterClockWise,	Qt::Key_D);
		setActionKeySequence(ActionToggleFullscreen,		Qt::Key_Return);
		setActionKeySequence(ActionExitFullscreen,			Qt::Key_Escape);
		setActionKeySequence(ActionResetView,			Qt::Key_Space);
		setActionKeySequence(ActionPreviousView,			QKeySequence(Qt::CTRL + Qt::Key_Left));
		setActionKeySequence(ActionNextView,			QKeySequence(Qt::CTRL + Qt::Key_Right));
		setActionKeySequence(ActionCloseView,			Qt::Key_Delete);
		setActionKeySequence(ActionCloseAllViews,			QKeySequence(Qt::SHIFT + Qt::Key_Delete));
		setActionKeySequence(ActionMotionModifier,			QKeySequence(Qt::CTRL + Qt::Key_Control, Qt::Key_Control), true); 	// The first correspond the press event, the second to the release.
		setActionKeySequence(ActionRotationModifier,		QKeySequence(Qt::SHIFT + Qt::Key_Shift, Qt::Key_Shift), true);		// (the same)
		setActionKeySequence(ActionNextSubWidget,			QKeySequence(Qt::ALT + Qt::Key_R));
		setActionKeySequence(ActionPreviousSubWidget,		QKeySequence(Qt::ALT + Qt::Key_E));
		setActionKeySequence(ActionTemporaryHideAllSubWidgets,	QKeySequence(Qt::ALT + Qt::Key_T));
		setActionKeySequence(ActionTemporaryUnhideAllSubWidgets,	QKeySequence(Qt::ALT + Qt::Key_Y));
		setActionKeySequence(ActionHideAllSubWidgets,		QKeySequence(Qt::ALT + Qt::Key_U));
	}

// MouseState :
	MouseState::MouseState(void)
	 :	functionMode(ModeCollection),
		wheelDelta(0.0f)
	{
		// Create the maps : 
		#define ADD_VECTOR( ID ) vectors.insert( ID , QPair<DataStatus, QPointF>(NotModified, QPointF(0.0, 0.0)));
			ADD_VECTOR( VectorLastLeftClick )
			ADD_VECTOR( VectorLastLeftClickGl )
			ADD_VECTOR( VectorLastLeftClickQuad )
			ADD_VECTOR( VectorLastLeftClickImage )

			ADD_VECTOR( VectorLastLeftPosition )
			ADD_VECTOR( VectorLastLeftPositionGl )
			ADD_VECTOR( VectorLastLeftPositionQuad )
			ADD_VECTOR( VectorLastLeftPositionImage )

			ADD_VECTOR( VectorLastLeftShift )
			ADD_VECTOR( VectorLastLeftShiftGl )
			ADD_VECTOR( VectorLastLeftShiftQuad )
			ADD_VECTOR( VectorLastLeftShiftImage )

			ADD_VECTOR( VectorLastLeftRelease )
			ADD_VECTOR( VectorLastLeftReleaseGl )
			ADD_VECTOR( VectorLastLeftReleaseQuad )
			ADD_VECTOR( VectorLastLeftReleaseImage )

			ADD_VECTOR( VectorLastLeftCompletedVector )
			ADD_VECTOR( VectorLastLeftCompletedVectorGl )
			ADD_VECTOR( VectorLastLeftCompletedVectorQuad )
			ADD_VECTOR( VectorLastLeftCompletedVectorImage )

			ADD_VECTOR( VectorLastRightClick )
			ADD_VECTOR( VectorLastRightClickGl )
			ADD_VECTOR( VectorLastRightClickQuad )
			ADD_VECTOR( VectorLastRightClickImage )

			ADD_VECTOR( VectorLastRightPosition )
			ADD_VECTOR( VectorLastRightPositionGl )
			ADD_VECTOR( VectorLastRightPositionQuad )
			ADD_VECTOR( VectorLastRightPositionImage )

			ADD_VECTOR( VectorLastRightShift )
			ADD_VECTOR( VectorLastRightShiftGl )
			ADD_VECTOR( VectorLastRightShiftQuad	)
			ADD_VECTOR( VectorLastRightShiftImage )

			ADD_VECTOR( VectorLastRightRelease )
			ADD_VECTOR( VectorLastRightReleaseGl )
			ADD_VECTOR( VectorLastRightReleaseQuad )
			ADD_VECTOR( VectorLastRightReleaseImage )

			ADD_VECTOR( VectorLastRightCompletedVector )
			ADD_VECTOR( VectorLastRightCompletedVectorGl )
			ADD_VECTOR( VectorLastRightCompletedVectorQuad )
			ADD_VECTOR( VectorLastRightCompletedVectorImage )

			ADD_VECTOR( VectorLastWheelUp )
			ADD_VECTOR( VectorLastWheelUpGl )
			ADD_VECTOR( VectorLastWheelUpQuad )
			ADD_VECTOR( VectorLastWheelUpImage )

			ADD_VECTOR( VectorLastWheelDown )
			ADD_VECTOR( VectorLastWheelDownGl )
			ADD_VECTOR( VectorLastWheelDownQuad )
			ADD_VECTOR( VectorLastWheelDownImage )
		#undef ADD_VECTOR

		#define ADD_COLOR( ID ) colors.insert( ID , QPair<DataStatus, QColor>(NotModified, QColor(Qt::black)));
			ADD_COLOR( ColorUnderLastLeftClick )
			ADD_COLOR( ColorUnderLastLeftPosition )
			ADD_COLOR( ColorUnderLastLeftRelease )
			ADD_COLOR( ColorUnderLastRightClick )
			ADD_COLOR( ColorUnderLastRightPosition )
			ADD_COLOR( ColorUnderLastRightRelease )
		#undef ADD_COLOR

		vectorIDs 	= vectors.keys();
		colorIDs 	= colors.keys();

		setFunctionMode(ModeCollection);
	}

	MouseState::~MouseState(void)
	{ }
	
	const QPointF& MouseState::invisibleGetVector(const VectorID& id) const
	{
		QMap<VectorID, QPair<DataStatus, QPointF> >::const_iterator it = vectors.find(id);

		if(it!=vectors.end())
			return it->second;
		else
			throw Exception("MouseState::getVector - Unknown VectorID (invisible access).", __FILE__, __LINE__);
	}

	const QColor& MouseState::invisibleGetColor(const ColorID& id) const
	{
		QMap<ColorID,  QPair<DataStatus, QColor> >::const_iterator it = colors.find(id);

		if(it!=colors.end())
			return it->second;
		else
			throw Exception("MouseState::getColor - Unknown ColorID (invisible access).", __FILE__, __LINE__);
	}

	bool MouseState::doesVectorRequireUpdate(const VectorID& id) const
	{
		return vectors[id].first == RequireUpdate;
	}

	bool MouseState::doesColorRequirepdate(const ColorID& id) const
	{
		return colors[id].first == RequireUpdate;
	}

	void MouseState::setVector(const VectorID& id, const QPointF& v, const bool requireUpdate)
	{
		if(vectors.contains(id))
		{
			vectors[id].first 	= (requireUpdate ? RequireUpdate : Modified);
			vectors[id].second 	= v;
		}
		else
			throw Exception("MouseState::setVector - Unknown VectorID.", __FILE__, __LINE__);
	}

	void MouseState::setColor(const ColorID& id, const QColor& c)
	{
		if(colors.contains(id))
		{
			colors[id].first 	= Modified;
			colors[id].second	= c;
		}
		else
			throw Exception("MouseState::setColor - Unknown ColorID.", __FILE__, __LINE__);
	}

	void MouseState::processEvent(QGraphicsSceneWheelEvent* event)
	{
		if(event->delta()!=0 && event->orientation()==Qt::Vertical)
		{
			wheelDelta += static_cast<float>(event->delta())/(8.0f*15.0f);

			// wheelSteps 	> 0 : away of the user
			// 		< 0 : toward the user

			if(wheelDelta>0)
				setVector(VectorLastWheelUp, event->scenePos(), true);
			else
				setVector(VectorLastWheelDown, event->scenePos(), true);

			event->accept();
			emit requestExternalUpdate();
		}
	}

	void MouseState::processEvent(QGraphicsSceneMouseEvent* event, const bool clicked, const bool moved, const bool released)
	{
		// event->button()  : The button which triggered the event (empty during drag).
		// event->buttons() : The buttons currently pressed (might not contain the previous in the case of a click).

		if(event->button()==Qt::LeftButton || (event->buttons() & Qt::LeftButton)!=0)
		{	
			if(clicked)
			{
				setVector(VectorLastLeftClick, event->buttonDownScenePos(Qt::LeftButton), true);
				setVector(VectorLastLeftPosition, event->scenePos(), true);
				//setVector(VectorLastLeftShift, QPointF(0.0, 0.0), true);

				if(getFunctionMode()==ModeMotion)
					emit mustSetMouseCursor(Qt::ClosedHandCursor);
			}			

			if(moved)
			{
				setVector(VectorLastLeftPosition, event->scenePos(), true);
				setVector(VectorLastLeftShift, event->scenePos() - event->lastScenePos(), true);
			}

			if(released)
			{
				setVector(VectorLastLeftRelease, event->scenePos(), true);
				setVector(VectorLastLeftCompletedVector, event->scenePos() - invisibleGetVector(VectorLastLeftClick), true);
				//setVector(VectorLastLeftShift, QPointF(0.0, 0.0), true);

				if(getFunctionMode()==ModeMotion)
					emit mustSetMouseCursor(Qt::OpenHandCursor);
			}

			if(clicked || moved || released)
				event->accept();
		}
		
		if(event->button()==Qt::RightButton || (event->buttons() & Qt::RightButton)!=0)
		{
			if(clicked)
			{
				setVector(VectorLastRightClick, event->buttonDownScenePos(Qt::RightButton), true);
				setVector(VectorLastRightPosition, event->scenePos(), true);
			}			

			if(moved)
			{
				setVector(VectorLastRightPosition, event->scenePos(), true);
				setVector(VectorLastRightShift, event->scenePos() - event->lastScenePos(), true);
			}

			if(released)
			{
				setVector(VectorLastRightRelease, event->scenePos(), true);
				setVector(VectorLastRightCompletedVector, event->scenePos() - invisibleGetVector(VectorLastRightClick), true);
			}

			if(clicked || moved || released)
				event->accept();
		}

		// Qt::MidButton ?

		// Require the widget to update all the coordinates and colors.
		if(event->isAccepted())
			emit requestExternalUpdate();
	}

	void MouseState::updateProcessCompleted(void)
	{
		emit updated();
	}

	void MouseState::clear(void)
	{
		for(QMap<VectorID, QPair<DataStatus, QPointF> >::Iterator it=vectors.begin(); it!=vectors.end(); it++)
		{
			it.value().first 	= NotModified;
			it.value().second 	= QPointF(0.0, 0.0);
		}

		for(QMap<ColorID, QPair<DataStatus, QColor> >::Iterator it=colors.begin(); it!=colors.end(); it++)
		{
			it.value().first 	= NotModified;
			it.value().second 	= Qt::black;
		}
	}

	const QList<MouseState::VectorID>& MouseState::getVectorIDs(void) const
	{
		return vectorIDs;
	}

	const QList<MouseState::ColorID>& MouseState::getColorIDs(void) const
	{
		return colorIDs;
	}

	bool MouseState::isVectorModified(const VectorID& id) const
	{
		return (vectors[id].first==Modified);
	}

	bool MouseState::isColorModified(const ColorID& id) const 
	{
		return (colors[id].first==Modified);
	}

	const QPointF& MouseState::getVector(const VectorID& id)
	{
		if(vectors.contains(id))
		{
			vectors[id].first = ((vectors[id].first==Modified) ? NotModified : vectors[id].first);
			return vectors[id].second;
		}
		else
			throw Exception("MouseState::getVector - Unknown VectorID.", __FILE__, __LINE__);
	}

	const QColor& MouseState::getColor(const ColorID& id)
	{
		if(colors.contains(id))
		{
			colors[id].first = ((colors[id].first==Modified) ? NotModified : colors[id].first);
			return colors[id].second;
		}
		else
			throw Exception("MouseState::getColor - Unknown ColorID.", __FILE__, __LINE__);
	}

	bool MouseState::isWheelDeltaModified(void) const
	{
		return (wheelDelta!=0.0f);
	}	
	
	float MouseState::getWheelDelta(void)
	{
		float c = wheelDelta;
		wheelDelta = 0.0f;
		return c;
	}

	const MouseState::FunctionMode& MouseState::getFunctionMode(void) const
	{
		return functionMode;
	}

	void MouseState::setFunctionMode(const FunctionMode& m)
	{
		functionMode = m;

		// See http://qt-project.org/doc/qt-4.8/qt.html#CursorShape-enum for more cursors shapes : 
		switch(m)
		{
			case ModeMotion :
				emit mustSetMouseCursor(Qt::OpenHandCursor);
				break;
			case ModeCollection :
				emit mustSetMouseCursor(Qt::CrossCursor);
				break;
			default : 
				throw Exception("MouseState::setFunctionMode - Unknown function mode.", __FILE__, __LINE__);
		}
	}

	MouseState::VectorID MouseState::validate(const VectorID& vID)
	{
		switch(vID)
		{
			case VectorLastLeftClick :
			case VectorLastLeftClickGl :
			case VectorLastLeftClickQuad :
			case VectorLastLeftClickImage :

			case VectorLastLeftPosition :
			case VectorLastLeftPositionGl :
			case VectorLastLeftPositionQuad :
			case VectorLastLeftPositionImage :

			case VectorLastLeftShift :
			case VectorLastLeftShiftGl :
			case VectorLastLeftShiftQuad :
			case VectorLastLeftShiftImage :

			case VectorLastLeftRelease :
			case VectorLastLeftReleaseGl :
			case VectorLastLeftReleaseQuad :
			case VectorLastLeftReleaseImage :

			case VectorLastLeftCompletedVector :
			case VectorLastLeftCompletedVectorGl :
			case VectorLastLeftCompletedVectorQuad :
			case VectorLastLeftCompletedVectorImage :

			case VectorLastRightClick :
			case VectorLastRightClickGl :
			case VectorLastRightClickQuad :
			case VectorLastRightClickImage :

			case VectorLastRightPosition :
			case VectorLastRightPositionGl :
			case VectorLastRightPositionQuad :
			case VectorLastRightPositionImage :

			case VectorLastRightShift :
			case VectorLastRightShiftGl :
			case VectorLastRightShiftQuad :
			case VectorLastRightShiftImage :

			case VectorLastRightRelease :
			case VectorLastRightReleaseGl :
			case VectorLastRightReleaseQuad :
			case VectorLastRightReleaseImage :

			case VectorLastRightCompletedVector :
			case VectorLastRightCompletedVectorGl :
			case VectorLastRightCompletedVectorQuad :
			case VectorLastRightCompletedVectorImage :

			case VectorLastWheelUp :
			case VectorLastWheelUpGl :
			case VectorLastWheelUpQuad :
			case VectorLastWheelUpImage :

			case VectorLastWheelDown :
			case VectorLastWheelDownGl :
			case VectorLastWheelDownQuad :
			case VectorLastWheelDownImage :
				return vID;

			default : 
				return InvalidVectorID;
		}
	}

	MouseState::ColorID MouseState::validate(const ColorID& cID)
	{
		switch(cID)
		{
			case ColorUnderLastLeftClick :
			case ColorUnderLastLeftPosition :
			case ColorUnderLastLeftRelease :
			case ColorUnderLastRightClick :
			case ColorUnderLastRightPosition :
			case ColorUnderLastRightRelease :
				return cID;
			
			default : 
				return InvalidColorID;
		}
	}

	MouseState::VectorID MouseState::getPixelVectorID(const VectorID& vID)
	{
		return validate(static_cast<VectorID>(vID - (vID % NumBasis)));
	}

	MouseState::BasisID MouseState::getVectorBasis(const VectorID& vID)
	{
		return static_cast<BasisID>(vID % NumBasis);
	}

	MouseState::ColorID MouseState::getCorrespondingColorID(const VectorID& vID)
	{
		return validate(static_cast<ColorID>(vID - (vID % NumBasis)));
	}

	bool MouseState::isBasisRelative(const BasisID& bID)
	{
		return (bID==PixelRelativeBasis) || (bID==GlRelativeBasis) || (bID==QuadRelativeBasis) || (bID==ImageRelativeBasis);
	}

// Shader code : 
	const std::string vertexShaderSource = 		"#version 130															\n"
							"																\n"
							"in vec2 vertexInput;														\n"
							"																\n"
							"uniform vec2	imageScale 	= vec2(1.0, 1.0),										\n"
							"		sceneScale 	= vec2(1.0, 1.0),										\n"
							"		viewCenter 	= vec2(0.0, 0.0),										\n"
							"		homothecyCenter	= vec2(0.0, 0.0);										\n"
							"uniform float	angle		= 0.0,												\n"
							"		homothecyScale	= 1.0,												\n"
							"		adaptationScale = 1.0;												\n"
							"																\n"
							"void main()															\n"
							"{																\n"
							"	// Generate rotation matrix : 												\n"
							"	mat2 rot 	= mat2(	 cos(angle), 	sin(angle), 									\n"
							"				-sin(angle), 	cos(angle));									\n"
							"																\n"
							"	vec2 tmpVertex	= vertexInput;												\n"
							"																\n"
							"	// Scale : 														\n"
							"	tmpVertex.x	= tmpVertex.x * imageScale.x;										\n"
							"	tmpVertex.y	= tmpVertex.y * imageScale.y;										\n"
							"																\n"
							"	// Rotate : 														\n"
							"	tmpVertex	= rot * tmpVertex;											\n"
							"																\n"
							"	// Translate : 														\n"
							"	tmpVertex	= tmpVertex + viewCenter;										\n"
							"																\n"
							"	// Homotothecy : 													\n"
							"	if(homothecyScale==0.0f) // Translation	only										\n"
							"		tmpVertex	= tmpVertex + homothecyCenter;									\n"
							"	else // Homothecy													\n"
							"		tmpVertex	= (tmpVertex - homothecyCenter) * homothecyScale + homothecyCenter;				\n"
							"																\n"
							"	// Scene Scaling :													\n"
							"	tmpVertex.x	= tmpVertex.x * sceneScale.x;										\n"
							"	tmpVertex.y	= tmpVertex.y * sceneScale.y;										\n"
							"																\n"
							"	// Adaptation Scaling :													\n"
							"	tmpVertex	= tmpVertex * adaptationScale;										\n"
							"																\n"
							"	// Write : 														\n"
							"	gl_Position 	= vec4(tmpVertex, 0.0, 1.0);										\n"
							"																\n"
							"	gl_TexCoord[0] 	= gl_MultiTexCoord0;											\n"
							"}																\n";

	const std::string fragmentShaderSource = 	"#version 130															\n"
							"																\n"
							"uniform sampler2D	viewTexture;												\n"
							"out     vec4 		displayOutput;												\n"
							"																\n"
							"void main()															\n"
							"{																\n"
							"	// Get the input data :													\n"
							"	displayOutput = textureLod(viewTexture, gl_TexCoord[0].st, 0.0);							\n"
							"}																\n";

// SceneWidget :
	SceneWidget::SceneWidget(MainWidget* _qvglParent, TopBar* _topBar, BottomBar* _bottomBar)
	 : 	qvglParent(_qvglParent),
		quad(NULL),
		shaderProgram(NULL),
		topBar(_topBar),
		bottomBar(_bottomBar)
	{
		// Activate focus via mouse click :
		//setFocusPolicy(Qt::ClickFocus);

		try
		{	
			quad = new GeometryInstance(GeometryPrimitives::StandardQuad(), GL_STATIC_DRAW_ARB);

			ShaderSource 	sourceVertex(vertexShaderSource),
					sourcePixel(fragmentShaderSource);

			HdlShader 	vertexShader(GL_VERTEX_SHADER, sourceVertex),
					pixelShader(GL_FRAGMENT_SHADER, sourcePixel);

			shaderProgram	= new HdlProgram( vertexShader, pixelShader);
		}
		catch(Exception& e)
		{
			delete quad;
			delete shaderProgram;
			quad = NULL;
			shaderProgram = NULL;
	
			// re-throw :
			throw e; 
		}

		// Bars (here because of the stretch signal) : 
		if(topBar!=NULL)
		{
			topBar->graphicsProxy = addWidget(topBar);
			QObject::connect(this, SIGNAL(sceneRectChanged(const QRectF&)), topBar, SLOT(stretch(const QRectF&)));
		}

		if(bottomBar!=NULL)
		{
			bottomBar->graphicsProxy = addWidget(bottomBar);
			QObject::connect(this, SIGNAL(sceneRectChanged(const QRectF&)), bottomBar, SLOT(stretch(const QRectF&)));
		}
	}

	SceneWidget::~SceneWidget(void)
	{
		delete quad;
		delete shaderProgram;
	}

	void SceneWidget::drawBackground(QPainter* painter, const QRectF& rect)
	{
		HdlTexture::unbind();

		glClearColor( 0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		View* currentView = qvglParent->getCurrentView();

		if(currentView!=NULL)
		{
			shaderProgram->use();

			float 	imageScale[2],
				sceneScale[2],
				adaptationScale;

			// Get the various scales : 
			currentView->getAspectRatioScaling(imageScale[0], imageScale[1]);
			qvglParent->getSceneRatioScaling(sceneScale[0], sceneScale[1]);
			adaptationScale = qvglParent->getAdaptationScaling(currentView->getImageRatio());

			// Load the data : 
			shaderProgram->modifyVar("imageScale", 	GL_FLOAT_VEC2,	imageScale);
			shaderProgram->modifyVar("sceneScale", 		GL_FLOAT_VEC2,	sceneScale);
			shaderProgram->modifyVar("adaptationScale", 	GL_FLOAT,	adaptationScale);
			shaderProgram->modifyVar("viewCenter",		GL_FLOAT_VEC2,	currentView->viewCenter);
			shaderProgram->modifyVar("homothecyCenter",	GL_FLOAT_VEC2,	currentView->homothecyCenter);
			shaderProgram->modifyVar("angle",		GL_FLOAT,	currentView->angle);
			shaderProgram->modifyVar("homothecyScale",	GL_FLOAT,	currentView->homothecyScale);

			currentView->prepareToDraw();

			quad->draw();

			//currentView->setAngle( currentView->getAngle() + 1.57079f); //0.174f);

			//std::cout << "Image      : " << currentView->getImageRatio() << std::endl;
			//std::cout << "Scene      : " << Parent->getSceneRatio() << std::endl;
			//std::cout << "Adaptation : " << adaptationScale << std::endl;
			//std::cout << "Angle      : " << currentView->getAngle() << std::endl;
		}		
		//else
		//	std::cout << "Nothing to draw!" << std::endl;

		/*{
			HdlProgram::stopProgram();
			HdlTexture::unbind();

			glBegin(GL_LINES);
				// X
				//glColor3f(1.0f,0.0f,1.0f);	// magenta
				//glVertex2f(-0.05f,-0.05f);
				//glVertex2f(5.0f,-0.05f);
				glColor3f(1.0f,0.0f,0.0f);	// red, ONE
				glVertex2f(0.0f,0.0f);
				glVertex2f(1.0f,0.0f);
				//glColor3f(1.0f,0.0f,1.0f);	// magenta
				//glVertex2f(0.05f,0.05f);
				//glVertex2f(0.5f,0.05f);

				// Y
				//glColor3f(0.0f,1.0f,1.0f);	// cyan
				//glVertex2f(-0.05f,-0.05f);
				//glVertex2f(-0.05f,5.0f);
				glColor3f(0.0f,1.0f,0.0f);	// green, ONE
				glVertex2f(0.0f,0.0f);
				glVertex2f(0.0f,1.0f);
				//glColor3f(0.0f,1.0f,1.0f);	// magenta
				//glVertex2f(0.05f,0.05f);
				//glVertex2f(0.05f,0.5f);
			glEnd();

			const float a = 0.3f;
			glBegin(GL_QUADS);
				glColor3f(1.0f,0.0f,0.0f);
				glVertex2f(-a,-a);
				glColor3f(0.0f,1.0f,0.0f);
				glVertex2f(-a,a);
				glColor3f(0.0f,0.0f,1.0f);
				glVertex2f(a,a);
				glColor3f(1.0f,1.0f,1.0f);
				glVertex2f(a,-a);
			glEnd();
		}*/
	}

	void SceneWidget::keyPressEvent(QKeyEvent* event)
	{
		QGraphicsScene::keyPressEvent(event);

		if(!event->isAccepted() && qvglParent!=NULL)
			qvglParent->getKeyboardState().keyPressed(event);

		//std::cout << "Pressed : " << qvglParent->getKeyboardState().getActionAssociatedToKey(event) << std::endl;
	}

	void SceneWidget::keyReleaseEvent(QKeyEvent* event)
	{
		QGraphicsScene::keyReleaseEvent(event);

		if(!event->isAccepted() && qvglParent!=NULL)
			qvglParent->getKeyboardState().keyReleased(event);

		//std::cout << "Released : " << qvglParent->getKeyboardState().getActionAssociatedToKey(event) << std::endl;
	}

	void SceneWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsScene::mouseMoveEvent(event);

		if(!event->isAccepted() && qvglParent!=NULL)
			qvglParent->getMouseState().processEvent(event, false, true, false);
	}

	void SceneWidget::wheelEvent(QGraphicsSceneWheelEvent* event)
	{
		QGraphicsScene::wheelEvent(event);

		if(!event->isAccepted() && qvglParent!=NULL)
			qvglParent->getMouseState().processEvent(event);
	}

	void SceneWidget::mousePressEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsScene::mousePressEvent(event);

		// TMP : 
		if(!event->isAccepted() && qvglParent!=NULL)
		{
			float xGl, yGl;
			qvglParent->toGlCoordinates(event->scenePos().x(), event->scenePos().y(), xGl, yGl, false);

			std::cout << "MousePressed - Gl Coordinates    : " << xGl << ", " << yGl << std::endl;

			float xQuad, yQuad;
			qvglParent->toQuadCoordinates(xGl, yGl, xQuad, yQuad, false);
			
			std::cout << "MousePressed - Quad Coordinates  : " << xQuad << ", " << yQuad << std::endl;

			float xImg, yImg;
			qvglParent->toImageCoordinates(xQuad, yQuad, xImg, yImg, false);
			std::cout << "MousePressed - Image Coordinates : " << xImg << ", " << yImg << std::endl;

			unsigned char red, green, blue;
			qvglParent->getColorAt(event->scenePos().x(), event->scenePos().y(), red, green, blue);
			std::cout << "MousePressed - Color             : (" << static_cast<int>(red) << "; " << static_cast<int>(green) << ", " << static_cast<int>(blue) << ")" << std::endl;
		}

		// Send the event :
		if(!event->isAccepted() && qvglParent!=NULL)
			qvglParent->getMouseState().processEvent(event, true, false, false);
	}

	void SceneWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsScene::mouseReleaseEvent(event);

		if(!event->isAccepted() && qvglParent!=NULL)
			qvglParent->getMouseState().processEvent(event, false, false, true);
	}

	void SceneWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsScene::mouseDoubleClickEvent(event);

		if(!event->isAccepted())
			event->accept();

		// TODO
			//qvglParent->getMouseState().processEvent(event);
		std::cout << "TODO - line " << __LINE__ << " : add double click event process." << std::endl;
	}

	/*void SceneWidget::addSubWidget(SubWidget* subWidget)
	{
		subWidget->graphicsProxy = addWidget(subWidget);

		// Connect : 
		// TODO : TO REMOVE
		QObject::con//nect(subWidget, SIGNAL(selected(SubWidget*)), this, SLOT(reorderItems(SubWidget*)));
	}

	void SceneWidget::putItemOnTop(QGraphicsProxyWidget* graphicsProxy)
	{
		if(graphicsProxy!=NULL)
		{
			QList<QGraphicsItem*> itemsList = items();

			qreal k = itemsList.count()-2;

			for(QList<QGraphicsItem*>::iterator it=itemsList.begin(); it!=itemsList.end(); it++)
			{
				if((*it)!=reinterpret_cast<QGraphicsItem*>(selectedWidget->graphicsProxy))
				{
					(*it)->setZValue(k);
					k--;
				}
			}

			// Raise to top : 
			graphicsProxy->setZValue(itemsList.count()-1.0);
		}
	}*/

// SceneViewWidget :
	SceneViewWidget::SceneViewWidget(MainWidget* _qvglParent, TopBar* _topBar, BottomBar* _bottomBar)
	 : 	contextWidget(NULL),
		sceneWidget(NULL),
		qvglParent(_qvglParent)
	{
		// Create the GL widget : 
		QGLFormat glFormat(QGL::DepthBuffer | QGL::DoubleBuffer);
		contextWidget = new ContextWidget(new QGLContext(glFormat), this);
		setViewport(contextWidget);

		// Create the scene : 
		sceneWidget = new SceneWidget(qvglParent, _topBar, _bottomBar);
		setScene(sceneWidget);

		// Other parameters : 	
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);	// For container catching.
		setViewportUpdateMode(QGraphicsView::FullViewportUpdate);	// Because GL has to redraw the complete area.
	}

	SceneViewWidget::~SceneViewWidget(void)
	{ }

	void SceneViewWidget::resizeEvent(QResizeEvent *event)
	{
		if(scene())
			scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));

		QGraphicsView::resizeEvent(event);
   	}

	void SceneViewWidget::addSubWidget(SubWidget* subWidget)
	{
		subWidget->graphicsProxy = sceneWidget->addWidget(subWidget);
	}

	SubWidget* SceneViewWidget::getUppermostSubWidget(const QList<SubWidget*>& list, bool onlyIfVisible) const
	{
		if(list.isEmpty())
			return NULL;
		else
		{
			SubWidget* highest = list.front();

			for(QList<SubWidget*>::const_iterator it=list.begin(); it!=list.end(); it++)
			{
				if((*it)->graphicsProxy!=NULL)
				{
					if(!highest->isVisible() && (*it)->isVisible() && onlyIfVisible)
						highest = (*it);
					else if((*it)->graphicsProxy->zValue() > highest->graphicsProxy->zValue() && ((*it)->isVisible() || !onlyIfVisible))
						highest = (*it);
				}
			}

			if(!highest->isVisible() && onlyIfVisible)
				return NULL;
			else			
				return highest;
		}
	}

	SubWidget* SceneViewWidget::getLowermostSubWidget(const QList<SubWidget*>& list, bool onlyIfVisible) const
	{
		if(list.isEmpty())
			return NULL;
		else
		{
			SubWidget* lowest = list.front();

			for(QList<SubWidget*>::const_iterator it=list.begin(); it!=list.end(); it++)
			{
				if((*it)->graphicsProxy!=NULL)
				{
					if(!lowest->isVisible() && (*it)->isVisible() && onlyIfVisible)
						lowest = (*it);
					else if((*it)->graphicsProxy->zValue() < lowest->graphicsProxy->zValue() && ((*it)->isVisible() || !onlyIfVisible))
						lowest = (*it);
				}
			}

			if(!lowest->isVisible() && onlyIfVisible)
				return NULL;
			else			
				return lowest;
		}
	}

	void SceneViewWidget::orderSubWidgetsList(QList<SubWidget*>& list, bool onlyIfVisible) const
	{
		if(onlyIfVisible)
		{
			// Remove non-visible widgets : 
			for(QList<SubWidget*>::iterator it=list.begin(); it!=list.end(); )
			{
				if(!(*it)->isVisible())
					it = list.erase(it);
				else
					it++;
			}
		}

		// Sort widget regarding their zValue : 
		for(int p=0; p<list.size(); p++)
		{
			for(int q=0; q<(list.size()-1); q++)
			{
				if(list[q+1]->graphicsProxy!=NULL && list[q]->graphicsProxy!=NULL)
				{
					if(list[q+1]->graphicsProxy->zValue() > list[q]->graphicsProxy->zValue())
						list.swap(q+1,q);
				}
			}
		}
	}

	void SceneViewWidget::putItemOnTop(QGraphicsProxyWidget* graphicsProxy)
	{
		if(graphicsProxy!=NULL)
		{
			QList<QGraphicsItem*> itemsList = sceneWidget->items();

			qreal k = itemsList.count()-2;

			for(QList<QGraphicsItem*>::iterator it=itemsList.begin(); it!=itemsList.end(); it++)
			{
				if((*it)!=graphicsProxy->graphicsItem())
				{
					(*it)->setZValue(k);
					k--;
				}
			}

			// Raise to top : 
			graphicsProxy->setZValue(itemsList.count()-1);
		}
	}

	void SceneViewWidget::putItemOnBottom(QGraphicsProxyWidget* graphicsProxy)
	{
		if(graphicsProxy!=NULL)
		{
			QList<QGraphicsItem*> itemsList = sceneWidget->items();

			qreal k = itemsList.count()-1;

			for(QList<QGraphicsItem*>::iterator it=itemsList.begin(); it!=itemsList.end(); it++)
			{
				if((*it)!=graphicsProxy->graphicsItem())
				{
					(*it)->setZValue(k);
					k--;
				}
			}

			// Raise to top : 
			graphicsProxy->setZValue(0.0);
		}
	}

	void SceneViewWidget::makeGLContextAvailable(void)
	{
		contextWidget->makeCurrent();
	}

	void SceneViewWidget::getColorAt(int x, int y, unsigned char& red, unsigned char& green, unsigned char& blue)
	{
		unsigned char rgb[3];

		glReadBuffer( GL_BACK );

		// Subtle point here : the frame buffer is verticaly flipped!
		glReadPixels(x, contextWidget->height()-(y+1), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, rgb);

		// Split : 
		red 	= rgb[0];
		green	= rgb[1];
		blue	= rgb[2];
	}

	void SceneViewWidget::getColorAt(int x, int y, QColor& c)
	{
		unsigned char rgb[3];

		glReadBuffer( GL_BACK );

		// Subtle point here : the frame buffer is verticaly flipped!
		glReadPixels(x, contextWidget->height()-(y+1), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, rgb);

		// Split : 
		c.setRed(rgb[0]);
		c.setGreen(rgb[1]);
		c.setBlue(rgb[2]);
	}

	void SceneViewWidget::update(void)
	{
		sceneWidget->update();
		//QGraphicsView::update(); // not required.
	}

// MainWidget :
	MainWidget::MainWidget(QWidget* parent)
	 :	QWidget(parent), 
		container(QBoxLayout::LeftToRight, this),
		sceneViewWidget(this, &topBar, &bottomBar),
		currentViewIndex(-1),
		opacityActiveSubWidget(0.8),
		opacityIdleSubWidget(0.2),
		opacityActiveBar(0.8),
		opacityIdleBar(0.4)
	{
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		container.addWidget(&sceneViewWidget);
		container.setMargin(0);
		container.setSpacing(0);

		// Reset bars views : 
		barSelected(&topBar);
		barSelected(&bottomBar);

		QObject::connect(&mouseState,		SIGNAL(requestExternalUpdate(void)),		this, SLOT(updateMouseStateData(void)));
		//QObject::connect(&sceneViewWidget, 	SIGNAL(requireContainerCatch(void)), 		this, SLOT(handleCatch(void)));			// Deprecated in current management of fullscreen.
		QObject::connect(&keyboardState,	SIGNAL(actionReceived(ActionID, bool)),	this, SLOT(processAction(ActionID, bool)));
		QObject::connect(&mouseState,		SIGNAL(updated(void)),				this, SLOT(performMouseAction(void)));
		QObject::connect(&mouseState,		SIGNAL(mustSetMouseCursor(Qt::CursorShape)),	this, SLOT(setMouseCursor(Qt::CursorShape)));
		QObject::connect(&topBar,		SIGNAL(selected(TopBar*)),			this, SLOT(barSelected(TopBar*)));
		QObject::connect(&topBar,		SIGNAL(changeViewRequest(View*)),		this, SLOT(viewRequireDisplay(View*)));
		QObject::connect(&topBar,		SIGNAL(showSubWidgetRequest(SubWidget*)),	this, SLOT(showSubWidget(SubWidget*)));
		QObject::connect(&topBar,		SIGNAL(temporaryHideAllSubWidgets(void)),	this, SLOT(temporaryHideAllSubWidgets()));
		QObject::connect(&topBar,		SIGNAL(hideAllSubWidgets(void)),		this, SLOT(hideAllSubWidgets()));
		QObject::connect(&bottomBar,		SIGNAL(selected(BottomBar*)),		this, SLOT(barSelected(BottomBar*)));
	}

	MainWidget::~MainWidget(void)
	{
		disconnect();

		for(QList<View*>::Iterator it=viewsList.begin(); it!=viewsList.end(); it++)
			(*it)->qvglParent = NULL;
	}

	void MainWidget::updateMouseStateData(void)
	{
		const QList<MouseState::VectorID>& vectorIDs = mouseState.getVectorIDs();

		for(QList<MouseState::VectorID>::const_iterator it=vectorIDs.begin(); it!=vectorIDs.end(); it++)
		{
			const MouseState::BasisID basisID = MouseState::getVectorBasis(*it);

			if( mouseState.doesVectorRequireUpdate(*it) && ((basisID==MouseState::PixelBasis) || (basisID==MouseState::PixelRelativeBasis))) // use the first one to populate the others : 
			{
				const bool isBasisRelative = (basisID==MouseState::PixelRelativeBasis);

				float 	xGl	= 0.0f, 
					yGl	= 0.0f, 
					xQuad	= 0.0f, 
					yQuad	= 0.0f,
					xImg	= 0.0f,
					yImg	= 0.0f;

				QPointF vPixel = mouseState.invisibleGetVector(*it);

				toGlCoordinates(vPixel.x(), vPixel.y(), xGl, yGl, isBasisRelative);
				toQuadCoordinates(xGl, yGl, xQuad, yQuad, isBasisRelative);
				toImageCoordinates(xQuad, yQuad, xImg, yImg, isBasisRelative);

				mouseState.setVector(*it, vPixel); // Clear the require update flag.
				mouseState.setVector(static_cast<MouseState::VectorID>(*it + MouseState::GlBasis), 	QPointF(xGl, yGl));
				mouseState.setVector(static_cast<MouseState::VectorID>(*it + MouseState::QuadBasis),	QPointF(xQuad, yQuad));
				mouseState.setVector(static_cast<MouseState::VectorID>(*it + MouseState::ImageBasis),	QPointF(xImg, yImg));

				// Update the corresponding color : 
				if(mouseState.getFunctionMode()==MouseState::ModeCollection)
				{
					MouseState::ColorID colorID = mouseState.getCorrespondingColorID(*it);
					QColor color;

					if(colorID!=MouseState::InvalidColorID)
					{
						getColorAt(vPixel.x(), vPixel.y(), color);

						mouseState.setColor(colorID, color);
					}

					// Give the information to TopBar in the case of last left position (ONLY) : 
					if((*it)==MouseState::VectorLastLeftPosition)
						topBar.updatePositionAndColor(QPointF(xImg, yImg), color);
				}

				//std::cout << "Updating from " << *it << " to " << static_cast<MouseState::VectorID>(*it + MouseState::GlBasis) << ", " << static_cast<MouseState::VectorID>(*it + MouseState::QuadBasis) << ", " << static_cast<MouseState::VectorID>(*it + MouseState::ImageBasis) << std::endl;
				//std::cout << "Vector : " << vPixel.x() << ", " << vPixel.y() << std::endl;

				// Need to update associated color : 

				// Set the read buffer : 
				// makeGLContextAvailable();
				//glReadBuffer( GL_BACK );

				// Subtle point here : the frame buffer is verticaly flipped!
				//glReadPixels(x, height()-(y+1), 1, 1, GL_RED, GL_UNSIGNED_BYTE, &id);
			}
		}

		mouseState.updateProcessCompleted();
	}

	void MainWidget::performMouseAction(void)
	{
		View* currentView = getCurrentView();

		if(currentView!=NULL && mouseState.getFunctionMode()==MouseState::ModeMotion)
		{
			if(mouseState.isVectorModified(MouseState::VectorLastLeftShiftGl))
			{
				QPointF v = mouseState.getVector(MouseState::VectorLastLeftShiftGl);

				// Go from Gl coordinates to Translation (before quad, do not use QuadBasis).

				// Adaptation scaling : 
				const float adaptationScale = getAdaptationScaling(currentView->getImageRatio());
				v.rx() /= adaptationScale;
				v.ry() /= adaptationScale;

				// Scene scaling : see toGlCoordinates

				// Homothecy : 
				if(currentView->homothecyScale!=0.0f)
				{
					v.rx() /= currentView->homothecyScale;
					v.ry() /= currentView->homothecyScale;
				}

				currentView->move(v.x(), v.y());
			}

			if(mouseState.isVectorModified(MouseState::VectorLastWheelUpGl) && mouseState.isWheelDeltaModified())
			{
				QPointF v = mouseState.getVector(MouseState::VectorLastWheelUpGl);

				// Go from Gl coordinates to Translation (before quad, do not use QuadBasis).

				// Adaptation scaling : 
				const float adaptationScale = getAdaptationScaling(currentView->getImageRatio());
				v.rx() /= adaptationScale;
				v.ry() /= adaptationScale;

				currentView->zoom(v.x(), v.y(), std::pow(1.2f, mouseState.getWheelDelta()));
			}

			if(mouseState.isVectorModified(MouseState::VectorLastWheelDownGl) && mouseState.isWheelDeltaModified())
			{
				QPointF v = mouseState.getVector(MouseState::VectorLastWheelDownGl);

				// Go from Gl coordinates to Translation (before quad, do not use QuadBasis).

				// Adaptation scaling : 
				const float adaptationScale = getAdaptationScaling(currentView->getImageRatio());
				v.rx() /= adaptationScale;
				v.ry() /= adaptationScale;

				currentView->zoom(v.x(), v.y(), std::pow(1.2f, mouseState.getWheelDelta()));
			}
		}
	}

	void MainWidget::setMouseCursor(Qt::CursorShape cursorShape)
	{
		setCursor(cursorShape);
		sceneViewWidget.setCursor(cursorShape);
		sceneViewWidget.viewport()->setCursor(cursorShape);
	}

	/*void MainWidget::handleCatch(void)
	{
		container.removeWidget(&sceneViewWidget);
		container.addWidget(&sceneViewWidget);
	}*/

	void MainWidget::viewRequireDisplay(View* view)
	{
		int idx = viewsList.indexOf(view);

		if(idx>=0 && idx<viewsList.size())
			changeCurrentView(idx);
	}

	void MainWidget::viewRequireDisplay(void)
	{
		// Get the emitter : 
		View* view = reinterpret_cast<View*>(QObject::sender());

		viewRequireDisplay(view);
	}

	void MainWidget::viewUpdated(View* view)
	{
		if(view==getCurrentView() && view!=NULL)
			sceneViewWidget.update();
	}

	void MainWidget::viewUpdated(void)
	{
		// Get the emitter : 
		View* view = reinterpret_cast<View*>(QObject::sender());

		viewUpdated(view);
	}

	void MainWidget::viewClosed(View* view)
	{
		int idx = viewsList.indexOf(view);

		// If this view is in the list : 
		if(idx!=-1)
		{
			// Remove this view from the list :
			viewsList.removeAt(idx);

			// Disconnect : 
			view->disconnect(this);
			view->qvglParent = NULL;
				
			// Change current view if needed :
			if(currentViewIndex==idx)
				changeCurrentView(currentViewIndex);

			// Update views list : 
			topBar.updateViewsList(viewsList);
		}
	}

	void MainWidget::viewClosed(void)
	{
		// Get the emitter : 
		View* view = reinterpret_cast<View*>(QObject::sender());
		
		viewClosed(view);
	}

	void MainWidget::subWidgetSelected(SubWidget* subWidget)
	{
		if(subWidgetsList.contains(subWidget) && subWidget->qvglParent==this && subWidget->graphicsProxy!=NULL)
		{
			// Change opacity of all other subWidgets : 
			for(QList<SubWidget*>::iterator it=subWidgetsList.begin(); it!=subWidgetsList.end(); it++)
				(*it)->setWindowOpacity(opacityIdleSubWidget);

			// Change the opacity of the bars : 
			topBar.setWindowOpacity(opacityIdleBar);
			bottomBar.setWindowOpacity(opacityIdleBar);

			// Change the opacity of the current subWidget : 
			subWidget->setWindowOpacity(opacityActiveSubWidget);

			// Raise the current subWidget : 
			sceneViewWidget.putItemOnTop(subWidget->graphicsProxy);
		}
	}

	void MainWidget::subWidgetSelected(void)
	{
		// Get the emitter : 
		SubWidget* subWidget = reinterpret_cast<SubWidget*>(QObject::sender());

		subWidgetSelected(subWidget);
	}

	void MainWidget::showSubWidget(SubWidget* subWidget)
	{
		if(!subWidget->isVisible())
			subWidget->show();		// implement the Show (1st part)
		else
			subWidgetSelected(subWidget);	// implement the raise (2nd part)
	}

	void MainWidget::showSubWidget(void)
	{
		// Get the emitter : 
		SubWidget* subWidget = reinterpret_cast<SubWidget*>(QObject::sender());

		showSubWidget(subWidget);
	}
	
	void MainWidget::hideSubWidget(SubWidget* subWidget)
	{
		if(subWidgetsList.contains(subWidget) && subWidget->qvglParent==this && subWidget->graphicsProxy!=NULL)
		{
			// Lower the current subWidget : 
			sceneViewWidget.putItemOnBottom(subWidget->graphicsProxy);

			// Raise the top bar : 
			sceneViewWidget.putItemOnTop(topBar.graphicsProxy);
		}
	}

	void MainWidget::hideSubWidget(void)
	{
		// Get the emitter : 
		SubWidget* subWidget = reinterpret_cast<SubWidget*>(QObject::sender());

		hideSubWidget(subWidget);
	}

	void MainWidget::subWidgetClosed(SubWidget* subWidget)
	{
		int idx = subWidgetsList.indexOf(subWidget);

		// If this view is in the list : 
		if(idx!=-1)
		{
			// Remove this view from the list :
			subWidgetsList.removeAt(idx);

			// Disconnect : 
			subWidget->disconnect(this);
			subWidget->qvglParent = NULL;

			// Update widgets list : 
			topBar.updateSubWidgetsList(subWidgetsList);
		}
	}

	void MainWidget::subWidgetClosed(void)
	{
		// Get the emitter : 
		SubWidget* subWidget = reinterpret_cast<SubWidget*>(QObject::sender());
		
		subWidgetClosed(subWidget);
	}

	void MainWidget::nextSubWidget(void)
	{
		// Get the ordered list of widgets : 
		QList<SubWidget*> list = subWidgetsList;
		sceneViewWidget.orderSubWidgetsList(list, true);

		if(list.count()>=2)
		{
			sceneViewWidget.putItemOnBottom(list[0]->graphicsProxy);
			sceneViewWidget.putItemOnTop(list[1]->graphicsProxy);

			// Change opacities : 
			for(QList<SubWidget*>::iterator it=subWidgetsList.begin(); it!=subWidgetsList.end(); it++)
				(*it)->setWindowOpacity(opacityIdleSubWidget);

			topBar.setWindowOpacity(opacityIdleBar);
			bottomBar.setWindowOpacity(opacityIdleBar);

			list[1]->setWindowOpacity(opacityActiveSubWidget);			
		}
	}

	void MainWidget::previousSubWidget(void)
	{
		// Get the ordered list of widgets : 
		QList<SubWidget*> list = subWidgetsList;
		sceneViewWidget.orderSubWidgetsList(list, true);

		if(list.count()>=2)
		{
			sceneViewWidget.putItemOnTop(list.back()->graphicsProxy);
			
			// Change opacities : 
			for(QList<SubWidget*>::iterator it=subWidgetsList.begin(); it!=subWidgetsList.end(); it++)
				(*it)->setWindowOpacity(opacityIdleSubWidget);

			topBar.setWindowOpacity(opacityIdleBar);
			bottomBar.setWindowOpacity(opacityIdleBar);

			list.back()->setWindowOpacity(opacityActiveSubWidget);			
		}
	}

	void MainWidget::temporaryHideAllSubWidgets(bool enabled)
	{
		for(QList<SubWidget*>::iterator it=subWidgetsList.begin(); it!=subWidgetsList.end(); it++)
			(*it)->temporaryHide(enabled);
	}

	void MainWidget::hideAllSubWidgets(void)
	{
		for(QList<SubWidget*>::iterator it=subWidgetsList.begin(); it!=subWidgetsList.end(); it++)
			(*it)->hide();
	}

	void MainWidget::barSelected(TopBar* bar)
	{
		if(bar==&topBar)
		{
			// Change opacity of all other subWidgets : 
			for(QList<SubWidget*>::iterator it=subWidgetsList.begin(); it!=subWidgetsList.end(); it++)
				(*it)->setWindowOpacity(opacityIdleSubWidget);

			// Change the opacity of the bars : 
			topBar.setWindowOpacity(opacityActiveBar);
			bottomBar.setWindowOpacity(opacityActiveBar);

			// Raise the bar : 
			sceneViewWidget.putItemOnTop(bottomBar.graphicsProxy);
			sceneViewWidget.putItemOnTop(topBar.graphicsProxy);		// Raise TOP on top.
		}	
	}

	void MainWidget::barSelected(BottomBar* bar)
	{
		if(bar==&bottomBar)
		{
			// Change opacity of all other subWidgets : 
			for(QList<SubWidget*>::iterator it=subWidgetsList.begin(); it!=subWidgetsList.end(); it++)
				(*it)->setWindowOpacity(opacityIdleSubWidget);

			// Change the opacity of the bars : 
			topBar.setWindowOpacity(opacityActiveBar);
			bottomBar.setWindowOpacity(opacityActiveBar);

			// Raise the bar : 
			sceneViewWidget.putItemOnTop(topBar.graphicsProxy);
			sceneViewWidget.putItemOnTop(bottomBar.graphicsProxy);		// Raise BOTTOM on top.
		}
	}

	KeyboardState& MainWidget::getKeyboardState(void)
	{
		return keyboardState;
	}

	MouseState& MainWidget::getMouseState(void)
	{
		return mouseState;
	}

	const KeyboardState& MainWidget::getKeyboardState(void) const
	{
		return keyboardState;
	}

	const MouseState& MainWidget::getMouseState(void) const
	{
		return mouseState;
	}

	View* MainWidget::getCurrentView(void) const
	{
		if(currentViewIndex>=0 && currentViewIndex<viewsList.size())
			return viewsList[currentViewIndex];
		else
			return NULL;
	}

	void MainWidget::changeCurrentView(int targetID)
	{
		currentViewIndex = std::min(std::max(targetID, 0), viewsList.size()-1);

		// Change title : 
		View* currentView = getCurrentView();

		if(currentView!=NULL)
		{
			topBar.setTitle(*currentView);

			// Show : 
			sceneViewWidget.update();
		}
		else
			topBar.setTitle("(No View)");
	}

	void MainWidget::getSceneRatioScaling(float& xSceneScale, float& ySceneScale) const
	{
		const float sceneRatio = getSceneRatio();

		xSceneScale = std::min(1.0f, 1.0f/sceneRatio);
		ySceneScale = std::min(1.0f, sceneRatio); 	// The minus sign set the orientation of the GL axis to be the same as Qt.
	}

	float MainWidget::getAdaptationScaling(const float& imageRatio) const
	{
		const float sceneRatio = getSceneRatio();

		if((sceneRatio<=1.0f && imageRatio>=1.0f) || (sceneRatio>=1.0f && imageRatio<=1.0f))	// Opposite aspect ratios : do not do anything.
			return 1.0f;
		else if(imageRatio<=1.0f)	// Portrait
		{
			if(sceneRatio>imageRatio)
				return std::max(sceneRatio, 1.0f/sceneRatio);
			else
				return std::max(imageRatio, 1.0f/imageRatio);
		}
		else //if(imageRatio>1.0f)	// Landscape
		{
			if(sceneRatio>imageRatio)
				return std::max(imageRatio, 1.0f/imageRatio);
			else
				return std::max(sceneRatio, 1.0f/sceneRatio);
		}
	}

	void MainWidget::toGlCoordinates(int x, int y, float& xGl, float& yGl, bool isRelative) const
	{
		QRectF rect = sceneRect();

		if(!isRelative)
		{
			xGl = (static_cast<float>(x) - rect.x()) * 2.0f / rect.width() - 1.0f;
			yGl = 1.0f - (static_cast<float>(y) - rect.y()) * 2.0f / rect.height();
		}
		else
		{
			xGl = static_cast<float>(x) * 2.0f / rect.width();
			yGl = - static_cast<float>(y) * 2.0f / rect.height();
		}

		// Scene scaling : 
		float xSceneScale, ySceneScale;
		getSceneRatioScaling(xSceneScale, ySceneScale);	

		xGl /= xSceneScale;
		yGl /= ySceneScale;
	}

	void MainWidget::toQuadCoordinates(const float& xGl, const float& yGl, float& xQuad, float& yQuad, bool isRelative, View* view) const
	{
		// Input coordinates are assumed relative in the window.

		if(view==NULL)
			view = getCurrentView();

		if(view==NULL)
		{
			xQuad = 0.0f;
			yQuad = 0.0f;
		}
		else if(!isRelative)
		{
			// Adaptation scaling : 
			const float adaptationScale = getAdaptationScaling(view->getImageRatio());
			float 	x = xGl / adaptationScale,
				y = yGl / adaptationScale;

			// Scene scaling : see toGlCoordinates
			
			// Homothecy :
			if( view->homothecyScale!=0.0f)
			{
				x = (x - view->homothecyCenter[0])/view->homothecyScale + view->homothecyCenter[0];
				y = (y - view->homothecyCenter[1])/view->homothecyScale + view->homothecyCenter[1];
			}
			// else : simplified.

			// Translation : 
			x -= view->viewCenter[0];
			y -= view->viewCenter[1];

			// Rotation : 
			const float 	c  = std::cos(view->angle),
					s  = std::sin(view->angle),
			 		x2 = c * x - s * y,
					y2 = s * x + c * y;

			// Image scaling : 
			float xImgScale, yImgScale;
			view->getAspectRatioScaling(xImgScale, yImgScale);

			xQuad = x2 / xImgScale;
			yQuad = y2 / yImgScale;
		}
		else
		{
			// Adaptation scaling : 
			const float adaptationScale = getAdaptationScaling(view->getImageRatio());
			float 	x = xGl / adaptationScale,
				y = yGl / adaptationScale;

			// Scene scaling : see toGlCoordinates

			// Homothecy : 
			if( view->homothecyScale!=0.0f)
			{
				x /= view->homothecyScale;
				y /= view->homothecyScale;
			}

			// Rotation : 
			const float 	c  = std::cos(view->angle),
					s  = std::sin(view->angle),
			 		x2 = c * x - s * y,
					y2 = s * x + c * y;

			// Image scaling : 
			float xImgScale, yImgScale;
			view->getAspectRatioScaling(xImgScale, yImgScale);

			xQuad = x2 / xImgScale;
			yQuad = y2 / yImgScale;
		}
	}

	void MainWidget::toImageCoordinates(const float& xQuad, const float& yQuad, float& xImg, float& yImg, bool isRelative, View* view) const
	{
		if(view==NULL)
			view = getCurrentView();

		if(view==NULL)
		{
			xImg = 0.0f;
			yImg = 0.0f;
		}
		else if(!isRelative)
		{
			xImg = (xQuad + 1.0f)*view->getFormat().getWidth()/2.0f;
			yImg = (1.0f - yQuad)*view->getFormat().getHeight()/2.0f;
		}
		else
		{
			xImg = xQuad*view->getFormat().getWidth()/2.0f;
			yImg = -yQuad*view->getFormat().getHeight()/2.0f;
		}
	}

	void MainWidget::addView(View* view)
	{
		if(!viewsList.contains(view) && view->qvglParent==NULL)
		{
			viewsList.append(view);

			view->qvglParent = this;

			// Connect actions : 
			QObject::connect(view, SIGNAL(requireDisplay()),	this, SLOT(viewRequireDisplay()));
			QObject::connect(view, SIGNAL(updated()),		this, SLOT(viewUpdated()));
			QObject::connect(view, SIGNAL(closed()), 		this, SLOT(viewClosed()));

			// Update views list : 
			topBar.updateViewsList(viewsList);
		}
	}

	void MainWidget::addSubWidget(SubWidget* subWidget)
	{
		if(!subWidgetsList.contains(subWidget) && subWidget->qvglParent==NULL)
		{
			sceneViewWidget.addSubWidget(subWidget);
			
			// Connect : 
			QObject::connect(subWidget, SIGNAL(selected(SubWidget*)), 		this, SLOT(subWidgetSelected(SubWidget*)));
			QObject::connect(subWidget, SIGNAL(showRequest(SubWidget*)),	this, SLOT(showSubWidget(SubWidget*)));
			QObject::connect(subWidget, SIGNAL(hideRequest(SubWidget*)),	this, SLOT(hideSubWidget(SubWidget*)));
			QObject::connect(subWidget, SIGNAL(destroyed(void)),			this, SLOT(subWidgetClosed(void)));

			// Save link : 
			subWidget->qvglParent = this;
			subWidgetsList.append(subWidget);

			// Update widgets list : 
			topBar.updateSubWidgetsList(subWidgetsList);

			// Show : 
			subWidget->show();
		}
	}

	float MainWidget::getSceneRatio(void) const
	{
		return sceneRect().width() / sceneRect().height();
	}

	QRectF MainWidget::sceneRect(void) const
	{
		return sceneViewWidget.sceneRect();
	}

	void MainWidget::getColorAt(int x, int y, unsigned char& red, unsigned char& green, unsigned char& blue)
	{
		MainWidget::sceneViewWidget.getColorAt(x, y, red, green, blue);
	}

	void MainWidget::getColorAt(int x, int y, QColor& c)
	{
		MainWidget::sceneViewWidget.getColorAt(x, y, c);
	}

	void MainWidget::processAction(ActionID action, bool takenBack)
	{
		View* currentView = getCurrentView();

		switch(action)
		{
			case ActionUp :
				if(currentView!=NULL) currentView->move(0.0f, -0.1f);
				break;
			case ActionDown :
				if(currentView!=NULL) currentView->move(0.0f, +0.1f);
				break;
			case ActionLeft :
				if(currentView!=NULL) currentView->move(-0.1f, 0.0f);
				break;
			case ActionRight :
				if(currentView!=NULL) currentView->move(+0.1f, 0.0f);
				break;
			case ActionZoomIn :
				if(currentView!=NULL)
				{
					float xCenter, yCenter;
					currentView->getViewCenter(xCenter, yCenter);

					// TODO - MISSING : SCREEN DEPENDENT SCALING
					std::cout << "TODO - line " << __LINE__ << " : add dependent screen scaling to solve bad centering bug." << std::endl;

					// Adaptation scaling : 
					//const float adaptationScale = getAdaptationScaling(currentView->getImageRatio());
					//xCenter *= adaptationScale;
					//yCenter *= adaptationScale;

					currentView->zoom(xCenter, yCenter, 1.2f);
				}
				break;
			case ActionZoomOut :
				if(currentView!=NULL)
				{
					float xCenter, yCenter;
					currentView->getViewCenter(xCenter, yCenter);

					// TODO - MISSING : SCREEN DEPENDENT SCALING
					std::cout << "TODO - line " << __LINE__ << " : add dependent screen scaling to solve bad centering bug." << std::endl;

					// Adaptation scaling : 
					//const float adaptationScale = getAdaptationScaling(currentView->getImageRatio());
					//xCenter *= adaptationScale;
					//yCenter *= adaptationScale;

					currentView->zoom(xCenter, yCenter, 1.0f/1.2f);
				}
				break;
			case ActionRotationClockWise :
				if(currentView!=NULL) currentView->rotate(-0.17453f);
				break;
			case ActionRotationCounterClockWise :
				if(currentView!=NULL) currentView->rotate(+0.17453f);
				break;
			case ActionToggleFullscreen :
				if(!sceneViewWidget.isFullScreen())
				{
					// Enter fullscreen : 
					sceneViewWidget.setParent(NULL);
					sceneViewWidget.showFullScreen();
					break;
				}
				// else : 
			case ActionExitFullscreen :
				// Re-attach :
				sceneViewWidget.setParent(this);
				sceneViewWidget.showNormal();
				container.removeWidget(&sceneViewWidget);
				container.addWidget(&sceneViewWidget);
				break;
			case ActionResetView :
				if(currentView!=NULL) currentView->reset();
				break;
			case ActionPreviousView :
				//currentViewIndex = std::max(currentViewIndex - 1, std::min(viewsList.size()-1, 0));
				changeCurrentView(currentViewIndex - 1);
				break;
			case ActionNextView :
				//currentViewIndex = std::min(currentViewIndex + 1, viewsList.size()-1);
				changeCurrentView(currentViewIndex + 1);
				break;
			case ActionCloseView :
				if(currentView!=NULL) currentView->close();
				break;
			case ActionCloseAllViews :
				break;
			case ActionMotionModifier :
				if(!takenBack)
					mouseState.setFunctionMode(MouseState::ModeMotion);
				else
					mouseState.setFunctionMode(MouseState::ModeCollection);
				break;
			case ActionRotationModifier :
				break;
			case ActionNextSubWidget :
				nextSubWidget();
				break;
			case ActionPreviousSubWidget :
				previousSubWidget();
				break;
			case ActionTemporaryHideAllSubWidgets :
				temporaryHideAllSubWidgets(true);
				break;
			case ActionTemporaryUnhideAllSubWidgets :
				temporaryHideAllSubWidgets(false);
				break;
			case ActionHideAllSubWidgets :
				hideAllSubWidgets();
				break;
			case NoAction :
				break;
			default : 
				throw Exception("MainWidget::processAction - Unknown action (code : " + to_string(action) + ").", __FILE__, __LINE__);
		}
	}

