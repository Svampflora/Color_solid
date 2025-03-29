#include "Gameplay.h"
#include "Endscreen.h"



template<typename T, typename Func>
void CheckConditionAndPerformAction(T value, Func action)
{
	if (value)
	{
		action();
	}
}

template <typename T>
void RenderObjects(const std::vector<T>& objects, const Texture& texture) noexcept
{
	for (const T& obj : objects)
	{
		obj.Render(texture);
	}
}

template <typename T>
void UpdateObjects(std::vector<T>& objects)  noexcept
{
	for (T& obj : objects)
	{
		obj.Update();
	}
}

template <typename Container, typename Predicate>
void remove_if(Container& container, Predicate predicate)
{
	container.erase(
		std::remove_if(container.begin(), container.end(), predicate),
		container.end());
}



std::unique_ptr<State> Play_screen::Update()
{
	if (IsKeyReleased(KEY_Q))
	{
		return std::make_unique<End_screen>();
	}


	return nullptr;
}

[[gsl::suppress(f.6)]]
Play_screen::Play_screen()
{

}

void Play_screen::Render() const noexcept
{

}
