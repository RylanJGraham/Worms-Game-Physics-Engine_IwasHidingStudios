#include "Fonts.h"

#include "Application.h"
#include "ModuleTextures.h"
#include "ModuleRender.h"

#include "Globals.h"
#include <string.h>
#include "Defs.h"

Fonts::Fonts(bool start_enabled) : Module(start_enabled)
{
	name.Create("Fonts_Module");
}

Fonts::~Fonts() 
{}

// Load new texture from file path
int Fonts::Load(const char* texture_path, const char* characters, uint rows) {
	int id = -1;

	if (texture_path == nullptr || characters == nullptr || rows == 0) {
		LOG("Could not load font");
		return id;
	}

	SDL_Texture* tex = app->textures->Load(texture_path);

	if (tex == nullptr || strlen(characters) >= MAX_FONT_CHARS) {
		LOG("Could not load font at %s with characters '%s'", texture_path, characters);
		return id;
	}

	id = 0;
	for (; id < MAX_FONTS; ++id)
		if (fonts[id].texture == nullptr)
			break;

	if (id == MAX_FONTS) {
		LOG("Cannot load font %s. Array is full (max %d).", texture_path, MAX_FONTS);
		return id;
	}

	Font& font = fonts[id];

	font.texture = tex;
	font.rows = rows;



	strcpy_s(fonts[id].table, MAX_FONT_CHARS, characters);
	font.totalLength = strlen(characters);
	font.columns = fonts[id].totalLength / rows;

	uint tex_w, tex_h;
	app->textures->GetSize(tex, tex_w, tex_h);
	font.char_w = tex_w / font.columns;
	font.char_h = tex_h / font.rows;

	LOG("Successfully loaded BMP font from %s", texture_path);

	return id;
}

void Fonts::UnLoad(int font_id) {
	if (font_id >= 0 && font_id < MAX_FONTS && fonts[font_id].texture != nullptr) {
		app->textures->Unload(fonts[font_id].texture);
		fonts[font_id].texture = nullptr;
		LOG("Successfully Unloaded BMP font_id %d", font_id);
	}
}

void Fonts::BlitText(int x, int y, int font_id, const char* text) const {
	if (text == nullptr || font_id < 0 || font_id >= MAX_FONTS || fonts[font_id].texture == nullptr) {
		LOG("Unable to render text with bmp font id %d", font_id);
		return;
	}

	const Font* font = &fonts[font_id];
	SDL_Rect spriteRect;
	uint len = strlen(text);

	spriteRect.w = font->char_w;
	spriteRect.h = font->char_h;

	for (uint i = 0; i < len; ++i) {

		uint charIndex = 0;

		// Find the location of the current character in the lookup table
		for (uint j = 0; j < font->totalLength; ++j) {
			if (font->table[j] == text[i]) {
				charIndex = j;
				break;
			}
		}

		// Retrieve the position of the current character in the sprite
		spriteRect.x = spriteRect.w * (charIndex % font->columns);
		spriteRect.y = spriteRect.h * (charIndex / font->columns);

		app->renderer->Blit(font->texture, x, y, &spriteRect, SDL_FLIP_NONE, 0.0f, false);

		// Advance the position where we blit the next character
		x += spriteRect.w;
	}
}