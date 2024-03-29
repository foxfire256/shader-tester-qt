#ifndef GL_WIDGET_HPP
#define GL_WIDGET_HPP

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glad/glad.h>

#include <QOpenGLWidget>
#include <Eigen/Core>
#include <Eigen/Geometry>

class mesh;
class shader_program2;

namespace fox
{
	class counter;
}

class gl_widget : public QOpenGLWidget
{
	Q_OBJECT
public:
	gl_widget(QWidget *parent = nullptr) = delete;
	gl_widget(const std::string &config_file, const std::string& data_root, QWidget *parent = nullptr);
	virtual ~gl_widget() override;

	std::unique_ptr<mesh> m;
	std::unique_ptr<shader_program2> sp;

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
	std::string data_root;
	int w, h;
	std::unique_ptr<fox::counter> fps_counter;
	double render_time;
	int frames, framerate;

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

	std::vector<float> vertices;
	std::vector<float> normals;

	GLuint default_vao;

	Eigen::Vector3f Ka, Ks, Kd;
	float shininess;

	std::unique_ptr<fox::counter> update_counter;

	float rot_vel;
};

#endif
