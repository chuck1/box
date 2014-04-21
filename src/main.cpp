/* 
 * - cleanly destroy actors and scenes (so that scene can be reset in-game)
 * - automate connection of camera (and other objects) to devices (via JSL::Master)
 * - cleanly exit (JSL read threads, glut windows)
 * - create terminal (via GUL)
 * - make command line revolve around objects (ex. command to change camera mode or target would be directed to the camera)
 *   - help command to print list of objects listening to terminal
 *   - another idea: have program listen to actual terminal (/dev/ttyX)
 * - focus tracking for windows to suspend signals when appropriate
 */

#include <functional>

#include <nebula/network/server.hpp>
#include <nebula/network/client.hpp>

#include <gru/master.hpp>
#include <gru/window/window.hpp>
#include <gru/renderable.hpp>
#include <gru/scene/desc.hpp>
#include <gru/scene/scene.hpp>
#include <gru/gui/object/object_factory.hpp>
#include <gru/gui/object/textview.hpp>

#include <nebula/actor/raw_factory.hpp>
#include <nebula/app.hpp>
#include <nebula/user.hpp>
#include <nebula/physics.hpp>
#include <nebula/scene/scene.hpp>
#include <nebula/simulation_callback.hpp>
#include <nebula/shape.hpp>
#include <nebula/control/rigid_body/control.hpp>
#include <nebula/actor/Rigid_Dynamic.hpp>


namespace box
{
	enum box
	{
		WINDOW_0,
		WINDOW_1,
		SCENE_0,
		LAYOUT_HOME,
		LAYOUT_GAME,
	};

	namespace layouts
	{
		namespace home
		{
			class local_game: public glutpp::gui::object::textview
			{
				public:
					virtual int	mouse_button_fun(int button, int action, int mods)
					{
						return 1;
					}
			};
			class network_game: public glutpp::gui::object::textview
			{
				public:
					virtual int	mouse_button_fun(int button, int action, int mods)
					{
						return 1;
					}
			};
		}
		namespace game
		{
			class exit: public glutpp::gui::object::textview
			{
				public:
					virtual int	mouse_button_fun(int button, int action, int mods)
					{
						return 1;
					}
			};
		}
	}
	class object_factory: public glutpp::gui::object::object_factory
	{
		public:
			virtual glutpp::gui::object::object_s	create(tinyxml2::XMLElement* element) {

				assert(element);

				std::shared_ptr<glutpp::gui::object::object> object;

				char const * buf = element->Attribute("type");
				assert(buf);

				if(strcmp(buf, "home_local_game") == 0)
				{
					object.reset(new layouts::home::local_game);
				}
				else if(strcmp(buf, "home_network_game") == 0)
				{
					object.reset(new layouts::home::network_game);
				}
				else if(strcmp(buf, "game_exit") == 0)
				{
					object.reset(new layouts::game::exit);
				}
				else
				{
					object = glutpp::gui::object::object_factory::create(element);
				}

				object->load_xml(element);

				return object;
			}
	};
}

std::shared_ptr<neb::app> app;

int	client_main(char const * addr, short unsigned int port) {
	
	app->create_window(600, 600, 200, 100, "box");
	
	app->reset_client(addr, port);
	
	app->loop();
	
	return 0;	
}
/*void	create_player(glutpp::window::window_s wnd, glutpp::scene::scene_s scene) {
	
	auto rigidbody = create_player_actor(scene);

	// so you can fire
	rigidbody->connect(wnd);

	// control
	neb::control::rigid_body::raw_s raw;
	
	app->create_window(600, 600, 200, 100, "box");
	app->create_window(600, 600, 200, 100, "box second");

	//rigidbody->create_control(raw);
	
	// user
	//std::shared_ptr<neb::user> user(new neb::user);	
	//user->set_control(control);
	//user->connect(wnd);
	
	app->activate_scene(box::WINDOW_0, box::SCENE_0);
	app->activate_scene(box::WINDOW_1, box::SCENE_0);
	//app->activate_layout(box::LAYOUT_GAME);

}*/
neb::actor::rigid_body::rigid_body_s create_player_actor(glutpp::scene::scene_s scene) {
	
	glutpp::actor::desc_s ad = scene->actors_deferred_[(char*)"player0"];
	assert(ad);

	auto actor = app->scenes_[0]->create_actor_local(ad);
	auto rigidbody = actor->to_rigid_body();
	
	return rigidbody;
}
void	create_player(glutpp::window::window_s wnd, glutpp::scene::scene_s scene) {
	
	auto rigidbody = create_player_actor(scene);

	// so you can fire
	rigidbody->connect(wnd);

	// control
	neb::control::rigid_body::raw_s raw;
	
	rigidbody->create_control(raw);
	
	//app->prepare();
	// user
	//std::shared_ptr<neb::user> user(new neb::user);	
	//user->set_control(control);
	//user->connect(wnd);
	

}
int	server_main(short unsigned int port) {

	app->reset_server(port);

	glutpp::scene::desc_s sd(new glutpp::scene::desc);
	sd->load("../scene.xml");

	{
		auto scene = app->load_scene_local(sd);
		assert(scene);

		app->load_layout(box::LAYOUT_HOME, "layout_home.xml");
		app->load_layout(box::LAYOUT_GAME, "layout_game.xml");
		
		
		glutpp::window::window_s wnd = app->create_window(600, 600, 200, 100, "box");
		
		
		app->activate_scene(0, 0);
		app->activate_layout(0, box::LAYOUT_GAME);

		create_player(wnd, scene);
		// vehicle
		//app->scenes_[box::SCENE_0]->create_vehicle();
	}
	app->loop();

	return 0;
}
int	main(int argc, char const ** argv)
{
	if(argc < 2)
	{
		printf("usage:\n");
		printf("    %s <mode> <port>\n", argv[0]);
		printf("    %s h\n", argv[0]);
		return 1;
	}

	if(strcmp(argv[1], "h") == 0)
	{
		printf("size\n");
		printf("glutpp::shape_desc    %i\n", (int)sizeof(glutpp::shape::desc));
		printf("neb::actor::desc      %i\n", (int)sizeof(glutpp::actor::desc));
		return 0;
	}

	if(argc < 3)
	{
		printf("usage: %s <mode> <port>\n", argv[0]);
		return 1;
	}

	neb::__physics.Init();

	glutpp::master::Global(glutpp::master_s(new glutpp::master));
	glutpp::master::Global()->object_factory_.reset(new box::object_factory);
	glutpp::master::Global()->raw_factory_.reset(new neb::actor::raw_factory);

	app.reset(new neb::app);
	app->init();



	if(strcmp(argv[1], "s") == 0)
	{
		return server_main(atoi(argv[2]));
	}
	else if(strcmp(argv[1], "c") == 0)
	{
		return client_main("127.0.0.1", atoi(argv[2]));
	}


	return 0;
}



