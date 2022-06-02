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
#include <nanogui/textbox.h>
#include <GLFW/glfw3.h>
#include <nanogui/vector.h>
#include <iostream>
#include <map>
#include "ini.h"
#include <filesystem>
#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

using nanogui::Vector2i;
namespace fs = std::filesystem;

// globals

#define TITLE "EasyRPG Savegame Manager"
#define INI "games.ini"

// ini:          id                     game name    path
typedef std::map<std::string, std::pair<std::string, std::string>> gamedata;
gamedata ini;

#ifdef EMSCRIPTEN
EM_ASYNC_JS(void, download_file, (const char* path), {
	console.log('Downloading ' + UTF8ToString(path) + '…');
});
#endif

int PopulateData(void* , const char* section, const char* name, const char* value) {
	std::string game, path;

#ifdef EMSCRIPTEN
	// full path
	path = "/easyrpg/" + std::string(section) + "/Save";
#else
	// relative for testing
	path = std::string(section);
#endif

	// check existence
	if (ini.find(section) != ini.end()) {
		std::pair<std::string, std::string> &val = ini[section];

		game = val.first;
		path = val.second;
	}

	// change value
	if (std::string(name) == "path")
		path = value;
	if (std::string(name) == "name")
		game = value;

	// save
	ini[section] = std::make_pair(game, path);

	return 1;
}

bool get_files(std::vector<std::string> &list, std::string dirpath) {
	list.clear();

	std::error_code ec;
	const fs::path dir{dirpath};
	for (auto const& entry : fs::directory_iterator{dir, ec}) {
		list.push_back(entry.path());
	}
	if (ec)
		std::cerr << "Problem listing " << dir << ": " << ec.message() << '\n';
	if (list.empty()) return false;

	std::sort(list.begin(), list.end());
	return true;
}

// app

class ESMW : public nanogui::Screen {
public:
	ESMW() : nanogui::Screen(Vector2i(800, 600), TITLE, false) {
		using namespace nanogui;

		// helpers
		auto tool_button_state = [=](bool enable) {
			for (int b = 0; b < actions->child_count() - 1; b++) {
				actions->child_at(b)->set_enabled(enable);
			}
		};

		auto update_save_blocks = [=](int id) {
			// delete old list, disable buttons
			while (saves->child_count() != 0)
				saves->remove_child_at(saves->child_count() - 1);
			tool_button_state(false);

			// return when not chosen
			if (id == 0) {
				perform_layout();
				return;
			}

			// change game
			selected_game = game_paths[id - 1];

			// get savefiles
			if (get_files(save_list, selected_game)) {
				for (const auto &s : save_list) {
					std::string basename = s.substr(s.find_last_of("/") + 1);
					Button *b = new Button(saves, basename);
					b->set_flags(Button::RadioButton);
					b->set_callback([=] {
						tool_button_state(true);
						selected_save = basename;
					});
				}

				perform_layout();
			}
		};

		// construct a window
		Window *window = new Window(this, TITLE);
		window->set_position(Vector2i(0, 0));
		window->set_layout(new GroupLayout());

		// top box with game list
		new Label(window, "Choose game:", "sans-bold");
		ComboBox *games = new ComboBox(window);

		// list of saves
		new Label(window, "Choose Savegame(s):", "sans-bold");
		saves = new Widget(window);
		saves->set_layout(new BoxLayout(Orientation::Vertical,
			Alignment::Maximum, 5, 0));

		new Label(window, "Actions:", "sans-bold");
		actions = new Widget(window);
		actions->set_layout(new BoxLayout(Orientation::Horizontal,
			Alignment::Maximum, 0, 2));

		PopupButton *ren = new PopupButton(actions, "Rename", FA_PEN);
		ren->set_tooltip("Rename the selected save.");
		Popup *popup = ren->popup();
		popup->set_anchor_pos(Vector2i(0, 0));
		popup->set_layout(new GroupLayout());
		new Label(popup, "New save ID:");
		auto *name_box = new IntBox<int>(popup);
		name_box->set_min_max_values(1, 15);
		name_box->set_value(1); // FIXME: insert current save id
		name_box->set_editable(true);
		name_box->set_format("(1[0-5]|[1-9])");
		name_box->set_spinnable(true);
		name_box->set_font_size(20);
		name_box->set_alignment(TextBox::Alignment::Right);
		Button *save = new Button(popup, "Save", FA_SAVE);
		save->set_callback([=] {
			std::cout << "not implemented: rename " << 
				selected_game << "/" << selected_save << " to " <<
				name_box->value() << '\n';
		});

		Button *down = new Button(actions, "Download", FA_DOWNLOAD);
		down->set_tooltip("Downloads all selected saves.");
		down->set_callback([=] {
			std::cout << "not implemented: download " << 
				selected_game << "/" << selected_save << '\n';
		});

		Button *del = new Button(actions, "Delete", FA_TRASH);
		del->set_tooltip("Deletes all selected saves.");
		del->set_callback([=] {
			auto dlg = new MessageDialog(this,
				MessageDialog::Type::Question, "Delete " + selected_save,
				"Do you really want to delete this savegame?", "Yes", "No", true);
			dlg->set_callback([=](int result) {
				if (result != 0) return; // No

				std::error_code ec;
				const fs::path sg{selected_game + "/" + selected_save};
				if(fs::remove_all(sg, ec)) {
					std::cout << "deleted " << selected_game << "/" << selected_save << '\n';
					// delete block
					update_save_blocks(games->selected_index());
				}
				else
					auto err = new MessageDialog(this,
						MessageDialog::Type::Warning, "Error",
						"Could not delete Savegame: " + ec.message());
			});
		});

		// exit
		Button *close = new Button(actions, "Close", FA_WINDOW_CLOSE);
		close->set_background_color(Color(0, 0, 255, 25));
		close->set_tooltip(":(");
		close->set_callback([=]() {
			set_visible(false);
		});

		// get games
		game_list = { "Choose" };
		for (auto &g : ini) {
			std::pair<std::string, std::string> &val = g.second;

			// any saves?
			if (get_files(save_list, val.second)) {
				game_list.push_back(val.first);
				game_paths.push_back(val.second);
			}
		}
		games->set_items(game_list);
		tool_button_state(false);

		// show saves
		games->set_callback(update_save_blocks);

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
	std::vector<std::string> game_list, game_paths, save_list;

	// makes callbacks easier
	Widget *saves, *actions;
};

// entry point

int main(int, char **) {
#ifdef EMSCRIPTEN
	emscripten_set_window_title(TITLE);

	// get ini
	emscripten_wget(INI, INI);
#endif

	// load ini
	int error = ini_parse(INI, PopulateData, nullptr);
	if (error > 0) {
		std::cerr << "Problem parsing " << INI << '\n';
		return -1;
	}

	for (auto &g : ini) {
		std::pair<std::string, std::string> &val = g.second;

#ifdef EMSCRIPTEN
		fs::create_directories(val.second);

		// Retrieve save directories from persistent storage before using
		EM_ASM({
			FS.mount(IDBFS, {}, UTF8ToString($0));
		}, val.second.c_str());
#else
		// simulate
		//std::cout << "Mounting " << g.first << ": " << val.second << '\n';
#endif
	}

#ifdef EMSCRIPTEN
	// sync files
	EM_ASM({
		FS.syncfs(true, function(err) {
			if(err) console.log(err);
		});
	});
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
