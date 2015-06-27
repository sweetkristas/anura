/*
	Copyright (C) 2003-2013 by Kristina Simpson <sweet.kristas@gmail.com>
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	   1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

#pragma once

#include "RenderFwd.hpp"
#include "SceneFwd.hpp"
#include "WindowManagerFwd.hpp"

#include "formula_callable.hpp"
#include "formula_callable_definition.hpp"

#include "xhtml.hpp"

namespace xhtml
{
	class DocumentObject : public game_logic::FormulaCallable
	{
	public:
		DocumentObject(const variant& v);
		variant write();

		void draw(const KRE::WindowPtr& wnd) const;
		void process();
		bool handleEvents(const SDL_Event& e);

		void surrenderReferences(GarbageCollector* collector);
	private:
		DECLARE_CALLABLE(DocumentObject);

		KRE::SceneGraphPtr scene_;
		KRE::SceneNodePtr root_;
		KRE::RenderManagerPtr rmanager_;
		int last_process_time_;
		
		xhtml::DocumentPtr doc_;
		xhtml::StyleNodePtr style_tree_;
		xhtml::DisplayListPtr display_list_;

		std::string doc_name_;
		std::string ss_name_;
	};
	
	typedef boost::intrusive_ptr<DocumentObject> DocumentObjectPtr;
}
