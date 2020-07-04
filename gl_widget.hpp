#ifndef GL_WIDGET_HPP
#define GL_WIDGET_HPP

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>

namespace fox
{
	class counter;
}
namespace fox::gfx
{
	class model_loader_obj;
}

class gl_widget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
	Q_OBJECT
public:
	gl_widget(QWidget *parent = nullptr);
	virtual ~gl_widget() override;

public slots:
	/**
	 * @brief Update the scene
	 */
	void animate(){}

signals:
	void update_fps(int fps);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

private:
	int w, h;
	fox::counter *fps_counter;
	double render_time;
	int frames, framerate;

	fox::gfx::model_loader_obj *obj;
};

#endif // GL_WIDGET_HPP
