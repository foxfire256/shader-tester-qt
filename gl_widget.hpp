#ifndef GL_WIDGET_HPP
#define GL_WIDGET_HPP

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <Eigen/Core>
#include <Eigen/Geometry>

class shader_program;
class mesh;

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
	gl_widget(QWidget *parent = nullptr) = delete;
	gl_widget(const std::string &config_file, QWidget *parent = nullptr);
	virtual ~gl_widget() override;

	std::unique_ptr<mesh> m;
	std::unique_ptr<shader_program> sp;

	std::unordered_map<std::string, float> u1f;
	std::unordered_map<std::string, std::array<float, 3>> u3fv;
	std::unordered_map<std::string, std::array<float, 4>> u4fv;

public slots:
	void uniform_changed_1f(const std::string &name, QString s);
	void uniform_changed_3fv(const std::string &name, int index, QString s);
	void uniform_changed_4fv(const std::string &name, int index, QString s);

signals:
	void update_fps(int fps);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void print_shader_info_log(GLuint shader_id);

private:
	std::string config_file;
	int w, h;
	fox::counter *fps_counter;
	double render_time;
	int frames, framerate;

	fox::gfx::model_loader_obj *obj;

	Eigen::Vector3f eye, target, up;
	Eigen::Affine3f V;
	Eigen::Projective3f P;
	// model matrix (specific to the model instance)
	Eigen::Projective3f MVP;
	Eigen::Affine3f M, MV;
	// TODO: should this be a pointer from the parent?
	// TODO: should this be Affine3f ?
	Eigen::Matrix3f normal_matrix;
	// more shader uniforms
	Eigen::Vector4f light_pos, color;
	Eigen::Vector3f La, Ls, Ld;
	Eigen::Vector3f rot, trans;
	float scale;

	GLuint fast_vertex_vbo;
	GLuint fast_normal_vbo;

	GLuint default_vao;

	Eigen::Vector3f Ka, Ks, Kd;
	float shininess;

	fox::counter *update_counter;

	float rot_vel;
};

#endif
