#include <cassert>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <array>
#include <vector>
#include <future>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Slider.H>
#include <FL/platform.H>

enum class Pitch {
	REST,
	C_NAT,
	C_SHARP,
	D_NAT,
	D_SHARP,
	E_NAT,
	F_NAT,
	F_SHARP,
	G_NAT,
	G_SHARP,
	A_NAT,
	A_SHARP,
	B_NAT,
};

struct Note_View {
	int32_t length = 0;
	Pitch pitch = Pitch::REST;
	int32_t octave = 0;
	int32_t speed = 0;
};

const Fl_Color NOTE_RED   = fl_rgb_color(217,   0,   0);
const Fl_Color NOTE_BLUE  = fl_rgb_color(  0, 117, 253);
const Fl_Color NOTE_GREEN = fl_rgb_color(  0, 165,   0);
const Fl_Color NOTE_BROWN = fl_rgb_color(124,  60,  25);

const Fl_Color NOTE_RED_LIGHT   = fl_lighter(NOTE_RED);
const Fl_Color NOTE_BLUE_LIGHT  = fl_lighter(NOTE_BLUE);
const Fl_Color NOTE_GREEN_LIGHT = fl_lighter(NOTE_GREEN);
const Fl_Color NOTE_BROWN_LIGHT = fl_lighter(NOTE_BROWN);

constexpr size_t NUM_WHITE_NOTES = 7;
constexpr size_t NUM_BLACK_NOTES = 5;

constexpr size_t NUM_NOTES_PER_OCTAVE = NUM_WHITE_NOTES + NUM_BLACK_NOTES;
constexpr size_t NUM_OCTAVES = 8;

constexpr int WHITE_KEY_WIDTH  = 150;
constexpr int WHITE_KEY_HEIGHT = 24;

constexpr int BLACK_KEY_WIDTH  = 100;
constexpr int BLACK_KEY_HEIGHT = 20;

constexpr int TICK_WIDTH = 3;
constexpr int TICKS_PER_STEP = 12;

struct Note_Key {
	int y, delta;
	Pitch pitch;
	bool white;
};

constexpr Note_Key NOTE_KEYS[NUM_NOTES_PER_OCTAVE] {
	{  0,  0, Pitch::B_NAT,   true },
	{  1,  0, Pitch::A_NAT,   true },
	{  2, +1, Pitch::G_NAT,   true },
	{  3, +1, Pitch::F_NAT,   true },
	{  4, -1, Pitch::E_NAT,   true },
	{  5, -1, Pitch::D_NAT,   true },
	{  6,  0, Pitch::C_NAT,   true },
	{  1,  0, Pitch::A_SHARP, false },
	{  3,  0, Pitch::G_SHARP, false },
	{  5,  0, Pitch::F_SHARP, false },
	{  8,  0, Pitch::D_SHARP, false },
	{ 10,  0, Pitch::C_SHARP, false },
};
constexpr size_t PITCH_TO_KEY_INDEX[NUM_NOTES_PER_OCTAVE] {
	6,  // C
	11, // C#
	5,  // D
	10, // D#
	4,  // E
	3,  // F
	9,  // F#
	2,  // G
	8,  // G#
	1,  // A
	7,  // A#
	0,  // B
};

constexpr char const *C_KEY_LABELS[] {
	"C1",
	"C2",
	"C3",
	"C4",
	"C5",
	"C6",
	"C7",
	"C8",
};

class Note_Box : public Fl_Box {
private:
	const Note_View &_note_view;
	int32_t _tick = 0;
public:
	Note_Box(const Note_View &n, int32_t t, int X, int Y, int W, int H, const char *l = nullptr);

	inline const Note_View &note_view() const { return _note_view; }
	inline int32_t tick() const { return _tick; }
};

class Key_Box : public Fl_Box {
public:
	using Fl_Box::Fl_Box;
};

class White_Key_Box : public Key_Box {
public:
	using Key_Box::Key_Box;
protected:
	void draw() override;
};

class Piano_Timeline;

class Piano_Keys : public Fl_Group {
private:
	std::array<Key_Box *, NUM_NOTES_PER_OCTAVE * NUM_OCTAVES> _keys;

	Pitch   _channel_1_pitch = Pitch::REST;
	int32_t _channel_1_octave = 0;
	Pitch   _channel_2_pitch = Pitch::REST;
	int32_t _channel_2_octave = 0;
	Pitch   _channel_3_pitch = Pitch::REST;
	int32_t _channel_3_octave = 0;
	Pitch   _channel_4_pitch = Pitch::REST;
	int32_t _channel_4_octave = 0;
public:
	Piano_Keys(int X, int Y, int W, int H, const char *l = nullptr);

	Piano_Keys(const Piano_Keys&) = delete;
	Piano_Keys& operator=(const Piano_Keys&) = delete;

	Piano_Timeline *parent() const { return (Piano_Timeline *)Fl_Group::parent(); }

	void calc_sizes();

	void set_key_color(Pitch pitch, int32_t octave, Fl_Color color);
	void update_key_colors();
	void reset_key_colors();

	inline void set_channel_1_pitch(Pitch p, int32_t o) { _channel_1_pitch = p; _channel_1_octave = o; }
	inline void set_channel_2_pitch(Pitch p, int32_t o) { _channel_2_pitch = p; _channel_2_octave = o; }
	inline void set_channel_3_pitch(Pitch p, int32_t o) { _channel_3_pitch = p; _channel_3_octave = o; }
	inline void set_channel_4_pitch(Pitch p, int32_t o) { _channel_4_pitch = p; _channel_4_octave = o; }

	void set_channel_pitch(int channel_number, Pitch p, int32_t o);
	void reset_channel_pitches();
};

class Piano_Roll;

class Piano_Timeline : public Fl_Group {
	friend class Piano_Roll;
private:
	Piano_Keys _keys;
	std::vector<Note_Box *> _channel_1_notes;
	std::vector<Note_Box *> _channel_2_notes;
	std::vector<Note_Box *> _channel_3_notes;
	std::vector<Note_Box *> _channel_4_notes;

	int32_t _cursor_tick = -1;
public:
	Piano_Timeline(int X, int Y, int W, int H, const char *l = nullptr);
	~Piano_Timeline() noexcept;

	Piano_Timeline(const Piano_Timeline&) = delete;
	Piano_Timeline& operator=(const Piano_Timeline&) = delete;

	Piano_Roll *parent() const { return (Piano_Roll *)Fl_Group::parent(); }

	void calc_sizes();

	void highlight_channel_1_tick(int32_t tick) { highlight_tick(_channel_1_notes, 1, tick, NOTE_RED_LIGHT); }
	void highlight_channel_2_tick(int32_t tick) { highlight_tick(_channel_2_notes, 2, tick, NOTE_BLUE_LIGHT); }
	void highlight_channel_3_tick(int32_t tick) { highlight_tick(_channel_3_notes, 3, tick, NOTE_GREEN_LIGHT); }
	void highlight_channel_4_tick(int32_t tick) { highlight_tick(_channel_4_notes, 4, tick, NOTE_BROWN_LIGHT); }

	void set_channel_1(const std::vector<Note_View> &notes) { set_channel(_channel_1_notes, 1, notes, NOTE_RED); }
	void set_channel_2(const std::vector<Note_View> &notes) { set_channel(_channel_2_notes, 2, notes, NOTE_BLUE); }
	void set_channel_3(const std::vector<Note_View> &notes) { set_channel(_channel_3_notes, 3, notes, NOTE_GREEN); }
	void set_channel_4(const std::vector<Note_View> &notes) { set_channel(_channel_4_notes, 4, notes, NOTE_BROWN); }

	void reset_note_colors();
private:
	void highlight_tick(std::vector<Note_Box *> &notes, int channel_number, int32_t tick, Fl_Color color);
	void set_channel(std::vector<Note_Box *> &channel, int channel_number, const std::vector<Note_View> &notes, Fl_Color color);
protected:
	void draw() override;
};

class Main_Window;

class Piano_Roll : public Fl_Scroll {
private:
	int32_t _tick = -1;
	bool _following = false;
	bool _continuous = true;
	bool _paused = false;
	int _ticks_per_step = TICKS_PER_STEP;

	Piano_Timeline _piano_timeline;

	std::vector<Note_View> _channel_1_notes;
	std::vector<Note_View> _channel_2_notes;
	std::vector<Note_View> _channel_3_notes;
	std::vector<Note_View> _channel_4_notes;

	int32_t _song_length = -1;
public:
	Piano_Roll(int X, int Y, int W, int H, const char *l = nullptr);
	~Piano_Roll() noexcept;

	Piano_Roll(const Piano_Roll&) = delete;
	Piano_Roll& operator=(const Piano_Roll&) = delete;

	Main_Window *parent() const { return (Main_Window *)Fl_Scroll::parent(); }

	inline int32_t tick() const { return _tick; }
	inline bool following() const { return _following; }
	inline bool paused() const { return _paused; }
	inline int ticks_per_step() const { return _ticks_per_step; }

	int white_key_height() const;
	int black_key_height() const;
	int octave_height() const;
	int note_row_height() const;
	int black_key_offset() const;
	int tick_width() const;

	void set_continuous_scroll(bool c) { _continuous = c; }

	void set_size(int W, int H);
	void set_timeline_width();

	void set_timeline();

	void build_note_view(int channel_number, std::vector<Note_View> &notes, Fl_Color color);

	int32_t get_last_note_x() const;

	void start_following();
	void unpause_following();
	void stop_following();
	void pause_following();
	void highlight_tick(int32_t t);
	void focus_cursor(bool center = false);
	void sticky_keys();

	void scroll_to_y_max();
	void scroll_to(int X, int Y);

	int scroll_x_max() const;
	int scroll_y_max() const;
private:
	static void scrollbar_cb(Fl_Scrollbar *sb, void *);
	static void hscrollbar_cb(Fl_Scrollbar *sb, void *);
};

static inline bool is_white_key(size_t i) {
	return !(i == 1 || i == 3 || i == 5 || i == 8 || i == 10);
}

Note_Box::Note_Box(const Note_View &n, int32_t t, int X, int Y, int W, int H, const char *l) : Fl_Box(X, Y, W, H, l), _note_view(n), _tick(t) {}

void White_Key_Box::draw() {
	draw_box();
	draw_label(x() + BLACK_KEY_WIDTH, y(), w() - BLACK_KEY_WIDTH, h());
}

Piano_Keys::Piano_Keys(int X, int Y, int W, int H, const char *l) : Fl_Group(X, Y, W, H, l) {
	resizable(nullptr);
	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			if (NOTE_KEYS[_x].white) {
				_keys[i] = new White_Key_Box(X, Y, 0, 0);
				_keys[i]->box(FL_BORDER_BOX);
				_keys[i]->color(FL_WHITE);
				if (NOTE_KEYS[_x].pitch == Pitch::C_NAT) {
					_keys[i]->label(C_KEY_LABELS[NUM_OCTAVES - _y - 1]);
				}
			}
			else {
				_keys[i] = new Key_Box(X, Y, 0, 0);
				_keys[i]->box(FL_BORDER_BOX);
				_keys[i]->color(FL_FOREGROUND_COLOR);
			}
		}
	}
	end();
	calc_sizes();
}

void Piano_Keys::calc_sizes() {
	const Piano_Roll *piano_roll = parent()->parent();
	const int white_key_height = piano_roll->white_key_height();
	const int black_key_height = piano_roll->black_key_height();
	const int octave_height = piano_roll->octave_height();
	const int note_row_height = piano_roll->note_row_height();
	const int black_key_offset = piano_roll->black_key_offset();

	int white_delta = 0, black_delta = 0;

	int y_top = _keys[0]->y();

	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		int y_pos = octave_height * (int)_y;
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			int delta = NOTE_KEYS[_x].delta;
			if (NOTE_KEYS[_x].white) {
				_keys[i]->resize(
					_keys[i]->x(),
					y_top + y_pos + NOTE_KEYS[_x].y * white_key_height + white_delta,
					WHITE_KEY_WIDTH,
					white_key_height + delta
				);
				white_delta += delta;
			}
			else {
				_keys[i]->resize(
					_keys[i]->x(),
					y_top + y_pos + NOTE_KEYS[_x].y * note_row_height + black_key_offset + black_delta,
					BLACK_KEY_WIDTH,
					black_key_height + delta
				);
				black_delta += delta;
			}
		}
	}

	size(w(), NUM_OCTAVES * octave_height);
}

void Piano_Keys::set_key_color(Pitch pitch, int32_t octave, Fl_Color color) {
	size_t _y = NUM_OCTAVES - octave;
	size_t _x = PITCH_TO_KEY_INDEX[(size_t)pitch - 1];
	size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
	if (_keys[i]->color() != color) {
		_keys[i]->color(color);
		_keys[i]->redraw();
	}
}

void Piano_Keys::update_key_colors() {
	reset_key_colors();
	if (_channel_1_pitch != Pitch::REST) {
		set_key_color(_channel_1_pitch, _channel_1_octave, NOTE_RED_LIGHT);
	}
	if (_channel_2_pitch != Pitch::REST) {
		set_key_color(_channel_2_pitch, _channel_2_octave, NOTE_BLUE_LIGHT);
	}
	if (_channel_3_pitch != Pitch::REST) {
		set_key_color(_channel_3_pitch, _channel_3_octave, NOTE_GREEN_LIGHT);
	}
	if (_channel_4_pitch != Pitch::REST) {
		set_key_color(_channel_4_pitch, _channel_4_octave, NOTE_BROWN_LIGHT);
	}
}

void Piano_Keys::reset_key_colors() {
	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			Fl_Color color = NOTE_KEYS[_x].white ? FL_WHITE : FL_FOREGROUND_COLOR;
			if (_keys[i]->color() != color) {
				_keys[i]->color(color);
				_keys[i]->redraw();
			}
		}
	}
}

void Piano_Keys::set_channel_pitch(int channel_number, Pitch p, int32_t o) {
	assert(channel_number >= 1 && channel_number <= 4);
	if (channel_number == 1) {
		set_channel_1_pitch(p, o);
	}
	else if (channel_number == 2) {
		set_channel_2_pitch(p, o);
	}
	else if (channel_number == 3) {
		set_channel_3_pitch(p, o);
	}
	else {
		set_channel_4_pitch(p, o);
	}
}

void Piano_Keys::reset_channel_pitches() {
	set_channel_1_pitch(Pitch::REST, 0);
	set_channel_2_pitch(Pitch::REST, 0);
	set_channel_3_pitch(Pitch::REST, 0);
	set_channel_4_pitch(Pitch::REST, 0);
	update_key_colors();
}

Piano_Timeline::Piano_Timeline(int X, int Y, int W, int H, const char *l) :
	Fl_Group(X, Y, W, H, l),
	_keys(X, Y, WHITE_KEY_WIDTH, H)
{
	resizable(nullptr);
	end();
}

Piano_Timeline::~Piano_Timeline() noexcept {
	remove(_keys);
	Fl_Group::clear();
}

void Piano_Timeline::calc_sizes() {
	const int octave_height = parent()->octave_height();
	const int note_row_height = parent()->note_row_height();
	const int tick_width = parent()->tick_width();

	const auto tick_to_x_pos = [&](int32_t tick) {
		return x() + WHITE_KEY_WIDTH + tick * tick_width;
	};
	const auto pitch_to_y_pos = [&](Pitch pitch, int32_t octave) {
		return y() + ((int)NUM_OCTAVES - octave) * octave_height + ((int)NUM_NOTES_PER_OCTAVE - (int)(pitch)) * note_row_height;
	};

	const auto resize_notes = [&](std::vector<Note_Box *> &notes) {
		for (Note_Box *note : notes) {
			note->resize(
				tick_to_x_pos(note->tick()),
				pitch_to_y_pos(note->note_view().pitch, note->note_view().octave),
				note->note_view().length * note->note_view().speed * tick_width,
				note_row_height
			);
		}
	};

	resize_notes(_channel_1_notes);
	resize_notes(_channel_2_notes);
	resize_notes(_channel_3_notes);
	resize_notes(_channel_4_notes);
}

void Piano_Timeline::reset_note_colors() {
	for (Note_Box *note : _channel_1_notes) {
		note->color(NOTE_RED);
	}
	for (Note_Box *note : _channel_2_notes) {
		note->color(NOTE_BLUE);
	}
	for (Note_Box *note : _channel_3_notes) {
		note->color(NOTE_GREEN);
	}
	for (Note_Box *note : _channel_4_notes) {
		note->color(NOTE_BROWN);
	}
}

void Piano_Timeline::highlight_tick(std::vector<Note_Box *> &notes, int channel_number, int32_t tick, Fl_Color color) {
	_keys.set_channel_pitch(channel_number, Pitch::REST, 0);
	for (Note_Box *note : notes) {
		const Note_View &view = note->note_view();
		int32_t t_left = note->tick();
		int32_t t_right = t_left + view.length * view.speed;
		if (t_left > tick) {
			return;
		}
		if (t_right > tick) {
			_keys.set_channel_pitch(channel_number, view.pitch, view.octave);
		}
		if (note->color() != color) {
			note->color(color);
			note->redraw();
		}
	}
}

void Piano_Timeline::set_channel(std::vector<Note_Box *> &channel, int channel_number, const std::vector<Note_View> &notes, Fl_Color color) {
	const int octave_height = parent()->octave_height();
	const int note_row_height = parent()->note_row_height();
	const int tick_width = parent()->tick_width();

	const auto tick_to_x_pos = [&](int32_t tick) {
		return x() + WHITE_KEY_WIDTH + tick * tick_width;
	};
	const auto pitch_to_y_pos = [&](Pitch pitch, int32_t octave) {
		return y() + ((int)NUM_OCTAVES - octave) * octave_height + ((int)NUM_NOTES_PER_OCTAVE - (int)(pitch)) * note_row_height;
	};

	begin();
	int32_t tick = 0;
	for (const Note_View &note : notes) {
		if (note.pitch != Pitch::REST) {
			Note_Box *box = new Note_Box(
				note,
				tick,
				tick_to_x_pos(tick),
				pitch_to_y_pos(note.pitch, note.octave),
				note.length * note.speed * tick_width,
				note_row_height
			);
			box->box(FL_BORDER_BOX);
			box->color(color);
			channel.push_back(box);
		}
		tick += note.length * note.speed;
	}
	end();

	// fix the keys as the last child
	Fl_Widget **a = (Fl_Widget **)array();
	if (a[children() - 1] != &_keys) {
		int i, j;
		for (i = j = 0; j < children(); j++) {
			if (a[j] != &_keys) {
				a[i++] = a[j];
			}
		}
		a[i] = &_keys;
	}
}

void Piano_Timeline::draw() {
	Fl_Color light_row = FL_LIGHT1;
	Fl_Color dark_row =FL_DARK2;
	Fl_Color row_divider = dark_row;
	Fl_Color col_divider = FL_DARK3;
	Fl_Color cursor_color = FL_MAGENTA;

	if (damage() & ~FL_DAMAGE_CHILD) {
		Piano_Roll *p = parent();
		const int note_row_height = p->note_row_height();
		const int tick_width = p->tick_width();
		const int ticks_per_step = p->ticks_per_step();

		int y_pos = y();
		for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
			for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
				if (is_white_key(_x)) {
					fl_rectf(x(), y_pos, w(), note_row_height, light_row);
				}
				else {
					fl_rectf(x(), y_pos, w(), note_row_height, dark_row);
				}
				if (_x == 0 || _x == 7) {
					fl_color(row_divider);
					fl_xyline(x(), y_pos - 1, x() + w());
					fl_xyline(x(), y_pos, x() + w());
				}
				y_pos += note_row_height;
			}
		}

		int x_pos = x() + WHITE_KEY_WIDTH;
		int time_step_width = tick_width * ticks_per_step;
		const size_t num_dividers = (w() - WHITE_KEY_WIDTH) / time_step_width + 1;
		for (size_t i = 0; i < num_dividers; ++i) {
			fl_color(col_divider);
			fl_yxline(x_pos - 1, y(), y() + h());
			x_pos += time_step_width;
		}

		_cursor_tick = p->tick();
		if (_cursor_tick != -1 && (parent()->following() || parent()->paused())) {
			_cursor_tick = _cursor_tick / ticks_per_step * ticks_per_step;
		}
		x_pos = x() + _cursor_tick * tick_width + WHITE_KEY_WIDTH;
		fl_color(cursor_color);
		fl_yxline(x_pos - 1, y(), y() + h());
		fl_yxline(x_pos, y(), y() + h());
	}
	draw_children();
}

Piano_Roll::Piano_Roll(int X, int Y, int W, int H, const char *l) :
	Fl_Scroll(X, Y, W, H, l),
	_piano_timeline(X, Y, W - scrollbar.w(), NUM_OCTAVES * octave_height())
{
	type(BOTH_ALWAYS);
	end();

	scrollbar.callback((Fl_Callback *)scrollbar_cb);
	hscrollbar.callback((Fl_Callback *)hscrollbar_cb);

	set_timeline();
}

Piano_Roll::~Piano_Roll() noexcept {
	remove(_piano_timeline);
}

int Piano_Roll::white_key_height() const {
	return WHITE_KEY_HEIGHT;
}

int Piano_Roll::black_key_height() const {
	return BLACK_KEY_HEIGHT;
}

int Piano_Roll::octave_height() const {
	return white_key_height() * NUM_WHITE_NOTES;
}

int Piano_Roll::note_row_height() const {
	return octave_height() / NUM_NOTES_PER_OCTAVE;
}

int Piano_Roll::black_key_offset() const {
	return note_row_height() / 2 - black_key_height() / 2;
}

int Piano_Roll::tick_width() const {
	return TICK_WIDTH;
}

void Piano_Roll::set_size(int W, int H) {
	if (W != w() || H != h()) {
		size(W, H);
		set_timeline_width();
		if (xposition() > scroll_x_max()) {
			scroll_to(scroll_x_max(), yposition());
			sticky_keys();
		}
		if (yposition() > scroll_y_max()) {
			scroll_to(xposition(), scroll_y_max());
		}
	}
}

void Piano_Roll::set_timeline_width() {
	_piano_timeline.w(std::max(WHITE_KEY_WIDTH + _song_length * tick_width(), w() - scrollbar.w()));
	int32_t last_note_x = get_last_note_x();
	int width = last_note_x + w() - scrollbar.w() - WHITE_KEY_WIDTH;
	if (width > _piano_timeline.w()) {
		_piano_timeline.w(width);
	}
}

void Piano_Roll::set_timeline() {
	_song_length = 3072;

	_piano_timeline.begin();
	build_note_view(1, _channel_1_notes, NOTE_RED);
	build_note_view(2, _channel_2_notes, NOTE_BLUE);
	build_note_view(3, _channel_3_notes, NOTE_GREEN);
	build_note_view(4, _channel_4_notes, NOTE_BROWN);
	_piano_timeline.end();

	_piano_timeline.set_channel_1(_channel_1_notes);
	_piano_timeline.set_channel_2(_channel_2_notes);
	_piano_timeline.set_channel_3(_channel_3_notes);
	_piano_timeline.set_channel_4(_channel_4_notes);

	set_timeline_width();
}

void Piano_Roll::build_note_view(
	int channel_number,
	std::vector<Note_View> &notes,
	Fl_Color color
) {
	int32_t tick = 0;

	Note_View note;

	while (tick < 3072 - 16) {
		note.octave = channel_number;
		note.speed = rand() % 4 + 1;
		note.length = rand() % 4 + 1;
		note.pitch = (Pitch)(rand() % 12 + 1);
		tick += note.length * note.speed;
		notes.push_back(note);
	}
}

int32_t Piano_Roll::get_last_note_x() const {
	const auto get_channel_last_note_x = [this](const std::vector<Note_Box *> &notes) {
		if (notes.size() > 0) {
			return notes.back()->x() - _piano_timeline.x();
		}
		return 0;
	};
	int32_t last_note_x = std::max({
		get_channel_last_note_x(_piano_timeline._channel_1_notes),
		get_channel_last_note_x(_piano_timeline._channel_2_notes),
		get_channel_last_note_x(_piano_timeline._channel_3_notes),
		get_channel_last_note_x(_piano_timeline._channel_4_notes),
	});
	return last_note_x;
}

void Piano_Roll::start_following() {
	_following = true;
	_paused = false;
	_piano_timeline.reset_note_colors();
	_piano_timeline._keys.reset_channel_pitches();
	if (_tick == -1) {
		scroll_to(0, yposition());
		sticky_keys();
	}
	redraw();
}

void Piano_Roll::unpause_following() {
	_following = true;
	_paused = false;
}

void Piano_Roll::stop_following() {
	_following = false;
	_paused = false;
	_tick = -1;
	_piano_timeline.reset_note_colors();
	_piano_timeline._keys.reset_channel_pitches();
	redraw();
}

void Piano_Roll::pause_following() {
	_following = false;
	_paused = true;
}

void Piano_Roll::highlight_tick(int32_t t) {
	if (_tick == t) return; // no change
	_tick = t;

	int scroll_x_before = xposition();

	_piano_timeline.highlight_channel_1_tick(_tick);
	_piano_timeline.highlight_channel_2_tick(_tick);
	_piano_timeline.highlight_channel_3_tick(_tick);
	_piano_timeline.highlight_channel_4_tick(_tick);
	_piano_timeline._keys.update_key_colors();
	_piano_timeline._keys.redraw();

	focus_cursor();
	if (
		_tick / ticks_per_step() * ticks_per_step() != _piano_timeline._cursor_tick ||
		xposition() != scroll_x_before
	) {
		redraw();
	}
}

void Piano_Roll::focus_cursor(bool center) {
	int x_pos = (_tick / ticks_per_step() * ticks_per_step()) * tick_width();
	if ((_following && _continuous) || x_pos > xposition() + w() - WHITE_KEY_WIDTH * 2 || x_pos < xposition()) {
		int scroll_pos = center ? x_pos + WHITE_KEY_WIDTH - w() / 2 : x_pos;
		scroll_to(std::min(std::max(scroll_pos, 0), scroll_x_max()), yposition());
		sticky_keys();
	}
}

void Piano_Roll::sticky_keys() {
	_piano_timeline._keys.position(0, _piano_timeline._keys.y());
}

void Piano_Roll::scroll_to_y_max() {
	scroll_to(xposition(), scroll_y_max());
}

void Piano_Roll::scroll_to(int X, int Y) {
	Fl_Scroll::scroll_to(X, Y);
}

int Piano_Roll::scroll_x_max() const {
	return _piano_timeline.w() - (w() - scrollbar.w());
}

int Piano_Roll::scroll_y_max() const {
	return _piano_timeline.h() - (h() - hscrollbar.h());
}

void Piano_Roll::scrollbar_cb(Fl_Scrollbar *sb, void *) {
	Piano_Roll *scroll = (Piano_Roll *)(sb->parent());
	scroll->scroll_to(scroll->xposition(), std::min(sb->value(), scroll->scroll_y_max()));
}

void Piano_Roll::hscrollbar_cb(Fl_Scrollbar *sb, void *) {
	Piano_Roll *scroll = (Piano_Roll *)(sb->parent());
	scroll->scroll_to(std::min(sb->value(), scroll->scroll_x_max()), scroll->yposition());
	scroll->sticky_keys();
	if (scroll->_following) {
		scroll->focus_cursor();
	}
	scroll->redraw();
}

class IT_Module {
private:
	int32_t _current_tick = 0;

	bool _playing = false;
	bool _paused = false;
	int _speed = 1;
public:
	IT_Module() {};

	IT_Module(const IT_Module&) = delete;
	IT_Module& operator=(const IT_Module&) = delete;

	bool ready()   const { return true; }
	bool playing() const { return _playing; }
	bool paused()  const { return _paused; }
	bool stopped() const { return !playing() && !paused(); }

	bool start()   { _paused = false; _playing = true;  return true; }
	bool stop()    { _paused = false; _playing = false; _current_tick = 0; return true; }
	bool pause()   { _paused = true;  _playing = false; return true; }
	void play();

	int32_t current_tick() const { return _current_tick; }

	int speed() const { return _speed; }
	void speed(int s) { _speed = s; }
};

void IT_Module::play() {
	if (!ready() || !playing()) return;

	std::size_t count = 1;
	_current_tick += _speed;
	if (_current_tick >= 3072) _current_tick = 0;

	if (count == 0) {
		stop();
	}
}

class Main_Window : public Fl_Double_Window {
private:
	Fl_Menu_Bar *_menu_bar;
	Fl_Menu_Item *_play_pause_mi;
	Fl_Menu_Item *_stop_mi;
	Fl_Menu_Item *_continuous_mi;
	Fl_Menu_Item *_full_screen_mi;
	Piano_Roll *_piano_roll;
	Fl_Group *_status_bar;
	Fl_Box *_speed_label;
	Fl_Slider *_speed_slider;
	Fl_Box *_fps_label;
	IT_Module _it_module;
	int32_t _tick = -1;
	bool _sync_requested = false;
	std::thread _audio_thread;
	std::mutex _audio_mutex;
	std::promise<void> _audio_kill_signal;
	int _frames = 0;
	int _frames_per_second = 0;
	time_t _frame_time = time(NULL);
public:
	Main_Window(int x, int y, int w, int h, const char *l = nullptr);
	~Main_Window();
	void resize(int X, int Y, int W, int H) override;
	inline bool continuous_scroll() const { return _continuous_mi && !!_continuous_mi->value(); }
	inline bool full_screen() const { return _full_screen_mi && !!_full_screen_mi->value(); }
	inline void continuous_scroll(bool c) { _continuous_mi->value(c);  continuous_cb(nullptr, this); }

	inline bool playing() { return _it_module.playing(); }
	inline bool paused()  { return _it_module.paused(); }
	inline bool stopped() { return _it_module.stopped(); }
protected:
	void draw() override;
private:
	void update_active_controls();
	void toggle_playback();
	void stop_playback();
	void start_audio_thread();
	void stop_audio_thread();
	void update_layout();

	static void speed_slider_cb(Fl_Widget *w);
	static void play_pause_cb(Fl_Widget *w, Main_Window *mw);
	static void stop_cb(Fl_Widget *w, Main_Window *mw);
	static void continuous_cb(Fl_Widget *w, Main_Window *mw);
	static void full_screen_cb(Fl_Widget *w, Main_Window *mw);
	static void playback_thread(Main_Window *mw, std::future<void> kill_signal);
	static void sync_cb(Main_Window *mw);
};

constexpr int MENU_BAR_HEIGHT = 21;
constexpr int STATUS_BAR_HEIGHT = 23;

#ifdef __APPLE__
#define FULLSCREEN_KEY FL_COMMAND + FL_SHIFT + 'f'
#else
#define FULLSCREEN_KEY FL_F + 11
#endif

Main_Window::Main_Window(int x, int y, int w, int h, const char *) : Fl_Double_Window(x, y, w, h, "Scroll Perf Test") {
	int wx = 0, wy = 0, ww = w, wh = h;

	_menu_bar = new Fl_Menu_Bar(wx, wy, ww, MENU_BAR_HEIGHT);
	wy += _menu_bar->h();
	wh -= _menu_bar->h();

	_status_bar = new Fl_Group(wx, h - STATUS_BAR_HEIGHT, ww, STATUS_BAR_HEIGHT);
	wh -= _status_bar->h();
	_status_bar->resizable(nullptr);
	_speed_label = new Fl_Box(wx, h - STATUS_BAR_HEIGHT, 50, STATUS_BAR_HEIGHT, "Speed:");
	_speed_slider = new Fl_Slider(wx + 50, h - STATUS_BAR_HEIGHT, 100, STATUS_BAR_HEIGHT);
	_speed_slider->type(FL_HORIZONTAL);
	_speed_slider->value(1.0);
	_speed_slider->bounds(1.0, 10.0);
	_speed_slider->user_data(this);
	_speed_slider->callback((Fl_Callback *)speed_slider_cb);
	_fps_label = new Fl_Box(wx + 150, h - STATUS_BAR_HEIGHT, 100, STATUS_BAR_HEIGHT);
	_fps_label->box(FL_FLAT_BOX);
	_status_bar->end();
	begin();

	_piano_roll = new Piano_Roll(wx, wy, ww, wh);

	Fl_Menu_Item menu_items[] = {
		{"&Play",              0,              0,                             0,    FL_SUBMENU,                     0, 0, 0, 0},
		{"&Play/Pause",        ' ',            (Fl_Callback *)play_pause_cb,  this, 0,                              0, 0, 0, 0},
		{"&Stop",              FL_Escape,      (Fl_Callback *)stop_cb,        this, FL_MENU_DIVIDER,                0, 0, 0, 0},
		{"&Continuous Scroll", '\\',           (Fl_Callback *)continuous_cb,  this, FL_MENU_TOGGLE | FL_MENU_VALUE, 0, 0, 0, 0},
		{},
		{"&View",              0,              0,                             0,    FL_SUBMENU,                     0, 0, 0, 0},
		{"Full &Screen",       FULLSCREEN_KEY, (Fl_Callback *)full_screen_cb, this, FL_MENU_TOGGLE,                 0, 0, 0, 0},
		{},
		{}
	};
	_menu_bar->copy(menu_items);
	_menu_bar->menu_end();

#define FIND_MENU_ITEM_CB(c) (const_cast<Fl_Menu_Item *>(_menu_bar->find_item((Fl_Callback *)(c))))
	_play_pause_mi = FIND_MENU_ITEM_CB(play_pause_cb);
	_stop_mi = FIND_MENU_ITEM_CB(stop_cb);
	_continuous_mi = FIND_MENU_ITEM_CB(continuous_cb);
	_full_screen_mi = FIND_MENU_ITEM_CB(full_screen_cb);
#undef FIND_MENU_ITEM_CB

	update_active_controls();
	update_layout();

	_piano_roll->scroll_to_y_max();
}

Main_Window::~Main_Window() {
	stop_audio_thread();
}

void Main_Window::resize(int X, int Y, int W, int H) {
	_menu_bar->size(W, MENU_BAR_HEIGHT);
	_piano_roll->position(0, MENU_BAR_HEIGHT);
	_piano_roll->set_size(W, H - MENU_BAR_HEIGHT - STATUS_BAR_HEIGHT);
	_status_bar->resize(0, H - STATUS_BAR_HEIGHT, W, STATUS_BAR_HEIGHT);
	Fl_Double_Window::resize(X, Y, W, H);
}

void Main_Window::draw() {
	Fl_Double_Window::draw();

	_frames += 1;
	time_t current_time = time(NULL);
	if (current_time > _frame_time) {
		_frames_per_second = (_frames_per_second + 3 * _frames / int(current_time - _frame_time)) / 4;
		_frame_time = current_time;
		_frames = 0;
	}

	char s[16];
	snprintf(s, sizeof(s), "FPS: %d", _frames_per_second);
	fl_color(FL_FOREGROUND_COLOR);
	fl_draw(s, _status_bar->x() + 160, _status_bar->y(), 100, _status_bar->h(), FL_ALIGN_LEFT);
}

void Main_Window::update_active_controls() {
	bool stopped = this->stopped();
	_play_pause_mi->activate();
	if (!stopped) {
		_stop_mi->activate();
	}
	else {
		_stop_mi->deactivate();
	}

	_menu_bar->update();
}

void Main_Window::toggle_playback() {
	stop_audio_thread();

	if (stopped()) {
		if (_it_module.ready() && _it_module.start()) {
			_piano_roll->start_following();
			start_audio_thread();
			update_active_controls();
		}
		else {
			return;
		}
	}
	else if (paused()) {
		if (_it_module.ready() && _it_module.start()) {
			_piano_roll->unpause_following();
			start_audio_thread();
			update_active_controls();
		}
		else {
			return;
		}
	}
	else { // if (playing())
		_it_module.pause();
		_piano_roll->pause_following();
		update_active_controls();
	}
}

void Main_Window::stop_playback() {
	stop_audio_thread();

	if (!_it_module.stopped()) {
		_it_module.stop();
		_tick = -1;
		_piano_roll->stop_following();
		update_active_controls();
	}
}

void Main_Window::start_audio_thread() {
	_audio_kill_signal = std::promise<void>();
	std::future<void> kill_future = _audio_kill_signal.get_future();
	_audio_thread = std::thread(&playback_thread, this, std::move(kill_future));
}

void Main_Window::stop_audio_thread() {
	if (_audio_thread.joinable()) {
		_audio_mutex.lock();
		_audio_kill_signal.set_value();
		_audio_thread.join();
		_audio_mutex.unlock();
	}
}

void Main_Window::update_layout() {
	_piano_roll->position(0, MENU_BAR_HEIGHT);
	_piano_roll->set_size(w(), h() - MENU_BAR_HEIGHT - STATUS_BAR_HEIGHT);
	size_range(
		WHITE_KEY_WIDTH * 3 + Fl::scrollbar_size(),
		MENU_BAR_HEIGHT + _piano_roll->octave_height() + Fl::scrollbar_size() + STATUS_BAR_HEIGHT,
		0,
		MENU_BAR_HEIGHT + _piano_roll->octave_height() * NUM_OCTAVES + Fl::scrollbar_size() + STATUS_BAR_HEIGHT
	);
}

void Main_Window::speed_slider_cb(Fl_Widget *w) {
	Main_Window *mw = (Main_Window *)w->user_data();
	int speed = (int)mw->_speed_slider->value();
	if (speed != mw->_it_module.speed()) {
		mw->_audio_mutex.lock();
		mw->_it_module.speed(speed);
		mw->_audio_mutex.unlock();
	}
}

void Main_Window::play_pause_cb(Fl_Widget *, Main_Window *mw) {
	mw->toggle_playback();
}

void Main_Window::stop_cb(Fl_Widget *, Main_Window *mw) {
	mw->stop_playback();
}

void Main_Window::continuous_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->set_continuous_scroll(mw->continuous_scroll());
	mw->redraw();
}

void Main_Window::full_screen_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->full_screen()) {
		mw->fullscreen();
	}
	else {
		mw->fullscreen_off();
	}
}

void Main_Window::playback_thread(Main_Window *mw, std::future<void> kill_signal) {
	int32_t tick = -1;
	while (kill_signal.wait_for(std::chrono::milliseconds(8)) == std::future_status::timeout) {
		if (mw->_audio_mutex.try_lock()) {
			IT_Module *mod = &mw->_it_module;
			if (mod && mod->playing()) {
				mod->play();
				int32_t t = mod->current_tick();
				if (tick != t) {
					tick = t;
					mw->_tick = t;
					if (!mw->_sync_requested) {
						Fl::awake((Fl_Awake_Handler)sync_cb, mw);
						mw->_sync_requested = true;
					}
				}
				mw->_audio_mutex.unlock();
			}
			else {
				mw->_tick = -1;
				if (!mw->_sync_requested) {
					Fl::awake((Fl_Awake_Handler)sync_cb, mw);
					mw->_sync_requested = true;
				}
				mw->_audio_mutex.unlock();
				break;
			}
		}
	}
}

void Main_Window::sync_cb(Main_Window *mw) {
	mw->_audio_mutex.lock();
	IT_Module *mod = &mw->_it_module;
	if (mod && mod->playing() && mw->_tick > 0) {
		mw->_piano_roll->highlight_tick(mw->_tick);
		mw->_status_bar->redraw();
	}
	else if (!mod || mod->stopped()) {
		mw->_tick = -1;
		mw->_piano_roll->stop_following();
		mw->update_active_controls();
	}
	mw->_sync_requested = false;
	mw->_audio_mutex.unlock();
}

static Main_Window *window = nullptr;

int main(int argc, char **argv) {
	window = new Main_Window(48, 48, 800, 600);
	Fl::lock();
	window->show();
	return Fl::run();
}
