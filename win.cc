#include "actions.h"
#include "prefs.h"
#include "stats.h"
#include "win.h"

void Stroke::draw(Cairo::RefPtr<Cairo::Surface> surface, int x, int y, int w, int h, bool invert) const {
	const Cairo::RefPtr<Cairo::Context> ctx = Cairo::Context::create (surface);
	ctx->set_line_width(2);
	x++; y++;
	w -= 2; h -= 2;
	bool first = true;
	for (std::vector<Point>::const_iterator j = points.begin(); j!=points.end();j++) {
		if (first) {
			ctx->move_to(w*j->x+x,h*j->y+y);
			first = false;
		}
		ctx->line_to(w*j->x+x,h*j->y+y);
		if (invert)
			ctx->set_source_rgba(1-j->time, j->time, 0,1);
		else
			ctx->set_source_rgba(0, j->time, 1-j->time,1);
		ctx->stroke();
		ctx->move_to(w*j->x+x,h*j->y+y);
	}
}

void Stroke::draw_svg(std::string filename) const {
	const int S = 32;
	const int B = 1;
	Cairo::RefPtr<Cairo::SvgSurface> s = Cairo::SvgSurface::create(filename, S, S);
	draw(s, B, B, S-2*B, S-2*B, false);
}


Glib::RefPtr<Gdk::Pixbuf> Stroke::draw_(int size) const {
	Glib::RefPtr<Gdk::Pixbuf> pb = drawEmpty_(size);
	// This is all pretty messed up
	// http://www.archivum.info/gtkmm-list@gnome.org/2007-05/msg00112.html
	Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create (pb->get_pixels(),
			Cairo::FORMAT_ARGB32, size, size, pb->get_rowstride());

	draw(surface, 0, 0, pb->get_width(), size);
	return pb;
}


Glib::RefPtr<Gdk::Pixbuf> Stroke::drawEmpty_(int size) {
	Glib::RefPtr<Gdk::Pixbuf> pb = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB,true,8,size,size);
	pb->fill(0xffffff00);
	return pb;
}

extern const char *gui_buffer;

int run_dialog(const char *str) {
	Glib::RefPtr<Gnome::Glade::Xml> xml = Gnome::Glade::Xml::create_from_buffer(gui_buffer, strlen(gui_buffer));
	Gtk::Dialog *dialog;
	xml->get_widget(str, dialog);
	int response = dialog->run();
//	printf("par: %x\n", dialog->get_parent());
	dialog->hide();
	return response;
}

Win::Win() :
	widgets(Gnome::Glade::Xml::create_from_buffer(gui_buffer, strlen(gui_buffer))),
	actions(new Actions(this)),
	prefs(new Prefs(this)),
	stats(new Stats(this)),
//	button_help("Show Help"),
	icon_queue(sigc::mem_fun(*this, &Win::on_icon_changed))
{
	current_icon = Stroke::trefoil();
	icon = Gtk::StatusIcon::create("");
	icon->signal_size_changed().connect(sigc::mem_fun(*this, &Win::on_icon_size_changed));
	icon->signal_activate().connect(sigc::mem_fun(*this, &Win::on_icon_click));
	void (Gtk::Menu::*f)(guint, guint32);
	f = &Gtk::Menu::popup;
	icon->signal_popup_menu().connect(sigc::mem_fun(menu, f));

	quit.connect(sigc::ptr_fun(&Gtk::Main::quit));

	WIDGET(Gtk::ImageMenuItem, menu_quit, Gtk::Stock::QUIT);
	menu.append(menu_quit);
	menu_quit.signal_activate().connect(sigc::ptr_fun(&Gtk::Main::quit));
	menu.show_all();

	widgets->get_widget("main", win);

	Gtk::Button* button_hide[3];
	widgets->get_widget("button_hide1", button_hide[0]);
	widgets->get_widget("button_hide2", button_hide[1]);
	widgets->get_widget("button_hide3", button_hide[2]);
	for (int i = 0; i < 3; i++)
		button_hide[i]->signal_clicked().connect(sigc::mem_fun(win, &Gtk::Window::hide));
}

Win::~Win() {
	delete actions;
	delete prefs;
	delete stats;
}

void Win::stroke_push(Ranking& r) {
	stats->stroke_push(r);
}

/*
void Win::on_help_toggled() {
	bool active = button_help.get_active();
	if (active)
		status.show();
	else
		status.hide();
	prefs().help.set(active);
	prefs().write();
}
*/

void Win::on_icon_changed(RStroke s) {
	current_icon = s;
	on_icon_size_changed(icon->get_size());
}

void Win::on_icon_click() {
	if (win->is_mapped())
		win->hide();
	else
		win->show();
}

bool Win::on_icon_size_changed(int size) {
	if (!current_icon)
		return true;
	icon->set(current_icon->draw(size));
	return true;
}