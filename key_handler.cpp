#include "key_handler.hpp"

#include <QEvent>
#include <QKeyEvent>

//------------------------------------------------------------------------------
key_handler::key_handler(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
bool key_handler::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() == QEvent::KeyPress)
	{
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);
		//printf("Ate key press %d\n", ke->key());
		//fflush(stdout);
		if(ke->key() == Qt::Key_Escape)
			emit quit();
		return true;
	}
	else
	{
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
}
