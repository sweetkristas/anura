/*
	Copyright (C) 2013-2014 by Kristina Simpson <sweet.kristas@gmail.com>
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	   1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgement in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "CanvasOGL.hpp"
#include "ShadersOGL.hpp"
#include "TextureOGL.hpp"

namespace KRE
{
	namespace
	{
		CanvasPtr& get_instance()
		{
			static CanvasPtr res = CanvasPtr(new CanvasOGL());
			return res;
		}
	}

	CanvasOGL::CanvasOGL()
	{
		handleDimensionsChanged();
	}

	CanvasOGL::~CanvasOGL()
	{
	}

	void CanvasOGL::handleDimensionsChanged()
	{
		mvp_ = glm::ortho(0.0f, static_cast<float>(width()), static_cast<float>(height()), 0.0f);
	}

	void CanvasOGL::blitTexture(const TexturePtr& texture, const rect& src, float rotation, const rect& dst, const Color& color) const
	{
		const float tx1 = texture->getNormalisedTextureCoordW<float>(src.x());
		const float ty1 = texture->getNormalisedTextureCoordH<float>(src.y());
		const float tx2 = texture->getNormalisedTextureCoordW<float>(src.w() == 0 ? texture->width() : src.x2());
		const float ty2 = texture->getNormalisedTextureCoordH<float>(src.h() == 0 ? texture->height() : src.y2());
		const float uv_coords[] = {
			tx1, ty1,
			tx2, ty1,
			tx1, ty2,
			tx2, ty2,
		};

		const float vx1 = float(dst.x());
		const float vy1 = float(dst.y());
		const float vx2 = float(dst.x2());
		const float vy2 = float(dst.y2());
		const float vtx_coords[] = {
			vx1, vy1,
			vx2, vy1,
			vx1, vy2,
			vx2, vy2,
		};

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3((vx1+vx2)/2.0f,(vy1+vy2)/2.0f,0.0f)) * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f,0.0f,1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(-(vx1+vx2)/2.0f,-(vy1+vy2)/2.0f,0.0f));
		glm::mat4 mvp = mvp_ * model * getModelMatrix();
		auto shader = OpenGL::ShaderProgram::defaultSystemShader();
		shader->makeActive();
		texture->bind();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));
		if(color != KRE::Color::colorWhite()) {
			shader->setUniformValue(shader->getColorUniform(), (color*getColor()).asFloatVector());
		} else {
			shader->setUniformValue(shader->getColorUniform(), getColor().asFloatVector());
		}
		shader->setUniformValue(shader->getTexMapUniform(), 0);
		// XXX the following line are only temporary, obviously.
		//shader->SetUniformValue(shader->GetUniformIterator("discard"), 0);
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords);
		glEnableVertexAttribArray(shader->getTexcoordAttribute()->second.location);
		glVertexAttribPointer(shader->getTexcoordAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, uv_coords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(shader->getTexcoordAttribute()->second.location);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::blitTexture(const TexturePtr& tex, const std::vector<vertex_texcoord>& vtc, float rotation, const Color& color)
	{
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1.0f));
		glm::mat4 mvp = mvp_ * model * getModelMatrix();
		auto shader = OpenGL::ShaderProgram::defaultSystemShader();
		shader->makeActive();
		tex->bind();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));
		if(color != KRE::Color::colorWhite()) {
			shader->setUniformValue(shader->getColorUniform(), (color*getColor()).asFloatVector());
		} else {
			shader->setUniformValue(shader->getColorUniform(), getColor().asFloatVector());
		}
		shader->setUniformValue(shader->getTexMapUniform(), 0);
		// XXX the following line are only temporary, obviously.
		//shader->SetUniformValue(shader->GetUniformIterator("discard"), 0);
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, offsetof(vertex_texcoord, vtx), &vtc[0]);
		glEnableVertexAttribArray(shader->getTexcoordAttribute()->second.location);
		glVertexAttribPointer(shader->getTexcoordAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, offsetof(vertex_texcoord, tc), &vtc[0]);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, vtc.size());

		glDisableVertexAttribArray(shader->getTexcoordAttribute()->second.location);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawSolidRect(const rect& r, const Color& fill_color, const Color& stroke_color, float rotation) const
	{
		rectf vtx = r.as_type<float>();
		const float vtx_coords[] = {
			vtx.x1(), vtx.y1(),
			vtx.x2(), vtx.y1(),
			vtx.x1(), vtx.y2(),
			vtx.x2(), vtx.y2(),
		};

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(vtx.mid_x(),vtx.mid_y(),0.0f)) * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f,0.0f,1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(-vtx.mid_x(),-vtx.mid_y(),0.0f));
		glm::mat4 mvp = mvp_ * model * getModelMatrix();
		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("simple");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		// Draw a filled rect
		shader->setUniformValue(shader->getColorUniform(), fill_color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// Draw stroke if stroke_color is specified.
		// XXX I think there is an easier way of doing this, with modern GL
		const float vtx_coords_line[] = {
			vtx.x1(), vtx.y1(),
			vtx.x2(), vtx.y1(),
			vtx.x2(), vtx.y2(),
			vtx.x1(), vtx.y2(),
			vtx.x1(), vtx.y1(),
		};
		shader->setUniformValue(shader->getColorUniform(), stroke_color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords_line);
		// XXX this may not be right.
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawSolidRect(const rect& r, const Color& fill_color, float rotation) const
	{
		rectf vtx = r.as_type<float>();
		const float vtx_coords[] = {
			vtx.x1(), vtx.y1(),
			vtx.x2(), vtx.y1(),
			vtx.x1(), vtx.y2(),
			vtx.x2(), vtx.y2(),
		};

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(vtx.mid_x(),vtx.mid_y(),0.0f)) * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f,0.0f,1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(-vtx.mid_x(),-vtx.mid_y(),0.0f));
		glm::mat4 mvp = mvp_ * model * getModelMatrix();
		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("simple");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		// Draw a filled rect
		shader->setUniformValue(shader->getColorUniform(), fill_color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawHollowRect(const rect& r, const Color& stroke_color, float rotation) const
	{
		rectf vtx = r.as_type<float>();
		const float vtx_coords_line[] = {
			vtx.x1(), vtx.y1(),
			vtx.x2(), vtx.y1(),
			vtx.x2(), vtx.y2(),
			vtx.x1(), vtx.y2(),
			vtx.x1(), vtx.y1(),
		};

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(vtx.mid_x(),vtx.mid_y(),0.0f)) * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f,0.0f,1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(-vtx.mid_x(),-vtx.mid_y(),0.0f));
		glm::mat4 mvp = mvp_ * model * getModelMatrix();

		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("simple");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		// Draw stroke if stroke_color is specified.
		// XXX I think there is an easier way of doing this, with modern GL
		shader->setUniformValue(shader->getColorUniform(), stroke_color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords_line);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawLine(const point& p1, const point& p2, const Color& color) const
	{
		const float vtx_coords_line[] = {
			static_cast<float>(p1.x), static_cast<float>(p1.y),
			static_cast<float>(p2.x), static_cast<float>(p2.y),
		};
		// This draws an aliased line -- consider making this a nicer unaliased line.
		glm::mat4 mvp = mvp_ * getModelMatrix();

		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("simple");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		shader->setUniformValue(shader->getColorUniform(), color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords_line);
		glDrawArrays(GL_LINES, 0, 2);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	std::ostream& operator<<(std::ostream& os, const glm::vec2& v)
	{
		os << "(" << v.x << "," << v.y << ")";
		return os;
	}

	void CanvasOGL::drawLines(const std::vector<glm::vec2>& varray, float line_width, const Color& color) const 
	{
		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("complex");
		shader->makeActive();
		shader->setUniformValue(shader->getMvUniform(), glm::value_ptr(getModelMatrix()));
		shader->setUniformValue(shader->getPUniform(), glm::value_ptr(mvp_));

		if(shader->getNormalAttribute() == shader->attributesIteratorEnd() || shader->getVertexAttribute() == shader->attributesIteratorEnd()) {
			return;
		}

		std::vector<glm::vec2> vertices;
		vertices.reserve(varray.size() * 2);
		std::vector<glm::vec2> normals;
		normals.reserve(varray.size() * 2);
		
		for(int n = 0; n != varray.size(); n += 2) {
			const float dx = varray[n+1].x - varray[n+0].x;
			const float dy = varray[n+1].y - varray[n+0].y;
			const glm::vec2 d1 = glm::normalize(glm::vec2(dy, -dx));
			const glm::vec2 d2 = glm::normalize(glm::vec2(-dy, dx));

			vertices.emplace_back(varray[n+0]);
			vertices.emplace_back(varray[n+0]);
			vertices.emplace_back(varray[n+1]);
			vertices.emplace_back(varray[n+1]);
						
			normals.emplace_back(d1);
			normals.emplace_back(d2);
			normals.emplace_back(d1);
			normals.emplace_back(d2);
		}

		static auto blur_uniform = shader->getUniformIterator("u_blur");
		shader->setUniformValue(blur_uniform, 2.0f);
		shader->setUniformValue(shader->getLineWidthUniform(), line_width);
		shader->setUniformValue(shader->getColorUniform(), color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glEnableVertexAttribArray(shader->getNormalAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, &vertices[0]);
		glVertexAttribPointer(shader->getNormalAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, &normals[0]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());
		glDisableVertexAttribArray(shader->getNormalAttribute()->second.location);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawLines(const std::vector<glm::vec2>& varray, float line_width, const std::vector<glm::u8vec4>& carray) const 
	{
		ASSERT_LOG(varray.size() == carray.size(), "Vertex and color array sizes don't match.");
		// This draws an aliased line -- consider making this a nicer unaliased line.
		glm::mat4 mvp = mvp_ * getModelMatrix();

		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("simple");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		shader->setUniformValue(shader->getLineWidthUniform(), line_width);
		shader->setUniformValue(shader->getColorUniform(), glm::value_ptr(glm::vec4(1.0f)));
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glEnableVertexAttribArray(shader->getColorAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, &varray[0]);
		glVertexAttribPointer(shader->getColorAttribute()->second.location, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &carray[0]);
		glDrawArrays(GL_LINES, 0, varray.size());
		glDisableVertexAttribArray(shader->getColorAttribute()->second.location);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawLineStrip(const std::vector<glm::vec2>& varray, float line_width, const Color& color) const 
	{
		// This draws an aliased line -- consider making this a nicer unaliased line.
		glm::mat4 mvp = mvp_ * getModelMatrix();

		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("simple");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		shader->setUniformValue(shader->getLineWidthUniform(), line_width);
		shader->setUniformValue(shader->getColorUniform(), color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, &varray[0]);
		glDrawArrays(GL_LINE_STRIP, 0, varray.size());
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawLineLoop(const std::vector<glm::vec2>& varray, float line_width, const Color& color) const 
	{
		// This draws an aliased line -- consider making this a nicer unaliased line.
		glm::mat4 mvp = mvp_ * getModelMatrix();

		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("simple");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		shader->setUniformValue(shader->getLineWidthUniform(), line_width);
		shader->setUniformValue(shader->getColorUniform(), color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, &varray[0]);
		glDrawArrays(GL_LINE_LOOP, 0, varray.size());
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawLine(const pointf& p1, const pointf& p2, const Color& color) const 
	{
		const float vtx_coords_line[] = {
			p1.x, p1.y,
			p2.x, p2.y,
		};
		// This draws an aliased line -- consider making this a nicer unaliased line.
		glm::mat4 mvp = mvp_ * getModelMatrix();

		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("simple");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		shader->setUniformValue(shader->getColorUniform(), color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords_line);
		glDrawArrays(GL_LINES, 0, 2);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawPolygon(const std::vector<glm::vec2>& varray, const Color& color) const 
	{
		// This draws an aliased line -- consider making this a nicer unaliased line.
		glm::mat4 mvp = mvp_ * getModelMatrix();

		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("simple");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		shader->setUniformValue(shader->getLineWidthUniform(), 1.0f);
		shader->setUniformValue(shader->getColorUniform(), color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, &varray[0]);
		glDrawArrays(GL_POLYGON, 0, varray.size());
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawSolidCircle(const point& centre, float radius, const Color& color) const 
	{
		drawSolidCircle(pointf(static_cast<float>(centre.x), static_cast<float>(centre.y)), radius, color);
	}

	void CanvasOGL::drawSolidCircle(const point& centre, float radius, const std::vector<glm::u8vec4>& color) const 
	{
		drawSolidCircle(pointf(static_cast<float>(centre.x), static_cast<float>(centre.y)), radius, color);
	}

	void CanvasOGL::drawHollowCircle(const point& centre, float radius, const Color& color) const 
	{
		drawHollowCircle(pointf(static_cast<float>(centre.x), static_cast<float>(centre.y)), radius, color);
	}

	void CanvasOGL::drawSolidCircle(const pointf& centre, float radius, const Color& color) const 
	{
		glm::mat4 mvp = mvp_ * getModelMatrix();

		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("circle");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		static auto radius_it = shader->getUniformIterator("outer_radius");
		shader->setUniformValue(radius_it, radius);
		static auto inner_radius_it = shader->getUniformIterator("inner_radius");
		shader->setUniformValue(inner_radius_it, 0.0f);
		shader->setUniformValue(shader->getColorUniform(), color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, glm::value_ptr(glm::vec2(static_cast<float>(centre.x), static_cast<float>(centre.y))));
		glDrawArrays(GL_POINTS, 0, 1);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawSolidCircle(const pointf& centre, float radius, const std::vector<glm::u8vec4>& color) const 
	{
		glm::mat4 mvp = mvp_ * glm::translate(glm::mat4(1.0f), glm::vec3(centre.x, centre.y, 0.0f)) * getModelMatrix();

		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("simple");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		// XXX figure out a nice way to do this with shaders.
		std::vector<glm::vec2> varray;
		varray.reserve(color.size());
		varray.emplace_back(0.0f, 0.0f);
		for(double angle = 0; angle < M_PI * 2.0; angle += (M_PI*2.0*4.0)/color.size()) {
				varray.emplace_back(radius*cos(angle), radius*sin(angle));
		}
		varray.emplace_back(varray[1]);

		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glEnableVertexAttribArray(shader->getColorAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, &varray[0]);
		glVertexAttribPointer(shader->getColorAttribute()->second.location, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &color[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 1);
		glDisableVertexAttribArray(shader->getColorAttribute()->second.location);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);

	}

	void CanvasOGL::drawHollowCircle(const pointf& centre, float radius, const Color& color) const 
	{
		glm::mat4 mvp = mvp_ * getModelMatrix();

		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("circle");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		static auto outer_radius_it = shader->getUniformIterator("outer_radius");
		shader->setUniformValue(outer_radius_it, radius);
		static auto inner_radius_it = shader->getUniformIterator("inner_radius");
		shader->setUniformValue(inner_radius_it, radius-1.0f);	// XXX replace 1.0f with line-width.
		shader->setUniformValue(shader->getColorUniform(), color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, glm::value_ptr(glm::vec2(static_cast<float>(centre.x), static_cast<float>(centre.y))));
		glDrawArrays(GL_POINTS, 0, 1);
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	void CanvasOGL::drawPoints(const std::vector<glm::vec2>& varray, float radius, const Color& color) const 
	{
		// This draws an aliased line -- consider making this a nicer unaliased line.
		glm::mat4 mvp = mvp_ * getModelMatrix();

		static OpenGL::ShaderProgramPtr shader = OpenGL::ShaderProgram::factory("simple");
		shader->makeActive();
		shader->setUniformValue(shader->getMvpUniform(), glm::value_ptr(mvp));

		static auto it = shader->getUniformIterator("point_size");
		shader->setUniformValue(it, radius);
		shader->setUniformValue(shader->getLineWidthUniform(), 1.0f);
		shader->setUniformValue(shader->getColorUniform(), color.asFloatVector());
		glEnableVertexAttribArray(shader->getVertexAttribute()->second.location);
		glVertexAttribPointer(shader->getVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, &varray[0]);
		glDrawArrays(GL_POINTS, 0, varray.size());
		glDisableVertexAttribArray(shader->getVertexAttribute()->second.location);
	}

	CanvasPtr CanvasOGL::getInstance()
	{
		return get_instance();
	}
}