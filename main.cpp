#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <iostream>
#include <cassert>


namespace Cells
{
	enum ID
	{
		Standart,
		Wall,
		Apple,
		SnakeSegment,
		SnakeHead,
	};
}

namespace Action
{
	enum ID
	{
		None,
		AddSegment,
		Move,
	};
}


class Cell //знает куда, как и что рисовать
{
public:
	typedef std::unique_ptr<Cell> Ptr;
	
	struct Context
	{
		Context(sf::RenderWindow& window, std::size_t& scale,  std::vector<sf::Texture>& textures, std::vector<sf::Color>& colors) //shapes - вектор с фигурами, я просто буду их двигать и рисовать
			: window(window)
			, scale(scale)
			, textures(textures)
			, colors(colors)
		{
		}
		Context();

		sf::RenderWindow& window;
		std::size_t& scale;
		std::vector<sf::Texture>& textures;
		std::vector<sf::Color>& colors;
	};

	Cell(sf::Vector2i pos, Cells::ID id, Context& context)
		: mContext(context)
		, mID(id)
	{
		mShape.setPosition(sf::Vector2f(pos.x * mContext.scale, pos.y * mContext.scale));
		mShape.setTexture(&mContext.textures[mID]);
	}
	void draw()
	{
		mContext.window.draw(mShape);
	}
	 Cells::ID returnID() 
	{
		 return mID;
	} //это по сути айди действия, которое обрабатывается в MapManage

	//Context getContext() { return mContext; }
private:
	Context& mContext;
	Cells::ID mID;
	sf::RectangleShape mShape;
};


class MapManage
{
public:
	MapManage(sf::RenderWindow& window, std::vector<sf::Texture>& textures, std::vector<sf::Color>& colors)
		: mWindow(window)
		, mTextures(textures)
		, mColors(colors)
	{
		createMap();
	}

	void draw()
	{
		for (std::size_t i = 0; i < 10; i++)
			for (std::size_t j = 0; j < 10; j++)
				Map[i][j]->draw();
	}

	void createMap()
	{
		std::vector<Cell*> vec;
		for (std::size_t i = 0; i < 10; i++)
			vec.push_back(new Cell(sf::Vector2i(i, 0), Cells::Standart, mContext));
		for (std::size_t i = 0; i < 10; i++)
			Map.push_back(vec);
	}



private:
	sf::RenderWindow& mWindow;
	std::vector<sf::Texture>& mTextures;
	std::vector<sf::Color>& mColors;
	std::vector<std::vector<Cell*>> Map;
	Cell::Context mContext;
};
int main()
{
	sf::RenderWindow window(sf::VideoMode(800, 800), "OneFileSFMLSnake");

	sf::CircleShape shape(25.f);
	sf::RectangleShape rectangle(sf::Vector2f(shape.getRadius() * 2, shape.getRadius() * 2));
	shape.setFillColor(sf::Color::Red);


	MapManage manage(window, textures, colors);
	manage.createMap();

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.key.code == sf::Keyboard::Escape)
				window.close();
		}




		window.clear();
		manage.draw();
		window.display();
	}

	return 0;
}
