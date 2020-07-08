#ifndef KEY_HANDLER_HPP
#define KEY_HANDLER_HPP

#include <QObject>

/**
 * @brief Handles keyboard events
 *
 * This should be installed at the main window level or it probably won't work.
 */
class key_handler : public QObject
{
	Q_OBJECT
public:
	explicit key_handler(QObject *parent = nullptr);

signals:
	void quit();

public slots:

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // KEY_HANDLER_WORLD_HPP
