#include "FadeToBlack.h"

#include "Application.h"
#include "ModuleRender.h"
#include "ModuleWindow.h"
#include "SDL/include/SDL_render.h"

FadeToBlack::FadeToBlack(bool start_enabled) : Module(start_enabled)
{
	name.Create("ModuleFadeToBlack");
}

FadeToBlack::~FadeToBlack()
{}

bool FadeToBlack::Start()
{
	uint w, h;
	w = app->window->screen_surface->w;
	h = app->window->screen_surface->h;

	screenRect = { 0, 0, (int)w * (int)app->window->GetScale(), (int)h * (int)app->window->GetScale() };

	LOG("Preparing Fade Screen");

	// Enable blending mode for transparency
	SDL_SetRenderDrawBlendMode(app->renderer->renderer, SDL_BLENDMODE_BLEND);
	return true;
}

update_status FadeToBlack::PreUpdate()
{
	// Exit this function if we are not performing a fade
	if (currentStep == Fade_Step::NONE) return UPDATE_CONTINUE;

	if (currentStep == Fade_Step::TO_BLACK)
	{
		++frameCount;
		if (frameCount >= maxFadeFrames)
		{
			moduleToDisable->Disable();
			moduleToEnable->Enable();

			currentStep = Fade_Step::FROM_BLACK;
		}
	}
	else
	{
		--frameCount;
		if (frameCount <= 0)
		{
			currentStep = Fade_Step::NONE;
		}
	}
	fadeRatio = (float)frameCount / (float)maxFadeFrames;
	LOG("FADE: %f", fadeRatio);

	

	return UPDATE_CONTINUE;
}

update_status FadeToBlack::Update()
{
	// Render the black square with alpha on the screen
	SDL_SetRenderDrawColor(app->renderer->renderer, 0, 0, 0, (Uint8)(fadeRatio * 255.0f));

	// Exit this function if we are not performing a fade
	if (currentStep == Fade_Step::NONE) return UPDATE_CONTINUE;

	SDL_RenderFillRect(app->renderer->renderer, NULL);

	return UPDATE_CONTINUE;
}

bool FadeToBlack::FadeBlack(Module* moduleToDisable, Module* moduleToEnable, float frames)
{
	bool ret = false;

	// If we are already in a fade process, ignore this call
	if (currentStep == Fade_Step::NONE)
	{
		currentStep = Fade_Step::TO_BLACK;
		frameCount = 0;
		maxFadeFrames = frames;

		this->moduleToDisable = moduleToDisable;
		this->moduleToEnable = moduleToEnable;

		ret = true;
	}

	return ret;
}