#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <string>
#include <memory>
#include <list>
#include <QMainWindow>

class QTimer;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;

class key_handler;
class gl_widget;

class main_window : public QMainWindow
{
	Q_OBJECT

public:
	main_window(QWidget *parent = nullptr) = delete;
	main_window(int win_w, int win_h, const std::string &config_file, QWidget *parent = nullptr);
	virtual ~main_window();

public slots:
	void update_fps(int fps);
	void quit();

private:
	int win_w;
	int win_h;
	std::string config_file;
	gl_widget *glw;
	key_handler *kh;
	QTimer *gfx_timer;
	QHBoxLayout *main_layout;
	QVBoxLayout *right_layout;
	QLabel *fps_lbl;
	QWidget *main_widget;

	std::list<std::shared_ptr<QWidget *>> uniform_widgets;
};

#endif
