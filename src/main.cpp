/*
*/

#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/window.h>
#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/combobox.h>
#include <nanogui/icons.h>
#include <nanogui/messagedialog.h>
#include <GLFW/glfw3.h>
#include <nanogui/vector.h>
#include <iostream>
#include <filesystem>
#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

using nanogui::Vector2i;

#define TITLE "EasyRPG Savegame Manager"

bool get_files(std::vector<std::string> &list, std::string dirpath) {
	using namespace std::filesystem;
	list.clear();

    std::error_code ec;
	const path dir{dirpath};
	for (auto const& entry : directory_iterator{dir, ec}) {
		list.push_back(entry.path());
	}
	if (ec)
		std::cerr << "Problem listing" << dir << ":" << ec.message() << '\n';
	if (list.empty()) return false;

	std::sort(list.begin(), list.end());
	return true;
}

class ESMW : public nanogui::Screen {
public:
	ESMW() : nanogui::Screen(Vector2i(800, 600), TITLE, false) {
		using namespace nanogui;

		// construct a window
		Window *window = new Window(this, TITLE);
		window->set_position(Vector2i(0, 0));
		window->set_layout(new GroupLayout());

		// top box with game list
		new Label(window, "Choose game:", "sans-bold");
		ComboBox *games = new ComboBox(window);

		// list of saves
		new Label(window, "Choose Savegame(s):", "sans-bold");
		Widget *saves = new Widget(window);
		saves->set_layout(new BoxLayout(Orientation::Vertical,
			Alignment::Maximum, 5, 0));

		new Label(window, "Actions:", "sans-bold");
		Widget *actions = new Widget(window);
		actions->set_layout(new BoxLayout(Orientation::Horizontal,
			Alignment::Maximum, 0, 2));

		Button *ren = new Button(actions, "Rename", FA_PEN);
		ren->set_enabled(false);
		ren->set_callback([&] {
			std::cout << "not implemented: rename " << 
				selected_game << "/" << selected_save << '\n';
		});

		Button *down = new Button(actions, "Download", FA_DOWNLOAD);
		down->set_enabled(false);
		down->set_callback([&] {
			std::cout << "not implemented: download " << 
				selected_game << "/" << selected_save << '\n';
		});

		Button *del = new Button(actions, "Delete", FA_TRASH);
		del->set_enabled(false);
		del->set_callback([&] {
			auto dlg = new MessageDialog(this,
				MessageDialog::Type::Warning, "Delete" + selected_save,
				"Do you really want to delete this savegame?", "Yes", "No", true);
			dlg->set_callback([&](int result) {
				if (result == 0) {
					// Yes
					std::cout << "not implemented: delete " << 
						selected_game << "/" << selected_save << '\n';
				}
			});
		});

		// exit
		Button *close = new Button(actions, "Close Window", FA_WINDOW_CLOSE);
		close->set_background_color(Color(0, 0, 255, 25));
		close->set_tooltip(":(");
		close->set_callback([&]() {
			set_visible(false);
		});

		// get games
		if (get_files(game_list, "Save")) {
			std::vector<std::string> short_names = { "Choose" };
			for (const auto &g : game_list) {
				std::string basename = g.substr(g.find_last_of("/") + 1);
				short_names.push_back(basename);
			}
			games->set_items(short_names);
		}

		// show saves
		games->set_callback([&, saves, ren, down, del](int id) {
			// delete old list
			while (saves->child_count() != 0)
				saves->remove_child_at(saves->child_count() - 1);
			// disable buttons
			ren->set_enabled(false);
			down->set_enabled(false);
			del->set_enabled(false);

			// return when not chosen
			if (id == 0) {
				perform_layout();
				return;
			}

			// change game
			selected_game = game_list.at(id - 1);

			// get games
			if (get_files(save_list, selected_game)) {
				for (const auto &s : save_list) {
					std::string basename = s.substr(s.find_last_of("/") + 1);
					Button *b = new Button(saves, basename);
					b->set_flags(Button::RadioButton);
					b->set_callback([&, ren, down, del, basename] {
						ren->set_enabled(true);
						down->set_enabled(true);
						del->set_enabled(true);
						selected_save = basename;
					});
				}

				perform_layout();
			}
		});

		perform_layout();
	}

	virtual bool keyboard_event(int key, int scancode, int action, int modifiers) {
		if (Screen::keyboard_event(key, scancode, action, modifiers))
			return true;
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			set_visible(false);
			return true;
		}
		return false;
	}

	virtual void draw(NVGcontext *ctx) {
		// Draw the user interface
		Screen::draw(ctx);
	}

private:
	std::string selected_game, selected_save;
	std::vector<std::string> game_list, save_list;

};

int main(int, char **) {
#ifdef EMSCRIPTEN
	emscripten_set_window_title(TITLE);

	// Retrieve save directory from persistent storage before using it
	EM_ASM(({
		FS.mkdir("easyrpg");
		FS.chdir("easyrpg");
		FS.mkdir("Save");
		FS.mount(IDBFS, {}, 'Save');
		FS.syncfs(true, function(err) {
			if(err) console.log(err);
		});
	}));
#endif

	try {
		nanogui::init();
		/* scoped variables */ {
			nanogui::ref<ESMW> app = new ESMW();
			app->draw_all();
			app->set_visible(true);
			nanogui::mainloop(1 / 60.f * 1000);
		}
		nanogui::shutdown();
	} catch (const std::runtime_error &e) {
		std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
		std::cerr << error_msg << '\n';
		return -1;
	}

#ifdef EMSCRIPTEN
	// minimize
	emscripten_set_canvas_element_size("#canvas", 0, 0);

	EM_ASM(({
		FS.syncfs(true, function(err) {
			if(err) console.log(err);
		});
	}));
#endif

	return 0;
}
