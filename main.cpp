#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <iostream>

class UniversalDad
{
public:
	UniversalDad(sf::Color color);
	virtual void doSomething(); //сделай что-то
	sf::Color mColor; //цвет клетки
private:

};
UniversalDad::UniversalDad(sf::Color color) : mColor(color)
{
}
void UniversalDad::doSomething()
{
}

class Segment : public UniversalDad
{
public:
	Segment(sf::Vector2i pos, sf::Color color = sf::Color::White);
	virtual void doSomething();
	sf::Vector2i mPos; //позиция сегмента
};
Segment::Segment(sf::Vector2i pos, sf::Color color)
	: mPos(pos)
	, UniversalDad(color)
{
}
void Segment::doSomething()
{
	std::cout << "YOU Loose" << std::endl;
}



class Head : public Segment
{
public:
	Head(sf::Vector2i pos, sf::Color color = sf::Color::Red);
	virtual void doSomething(std::function<void()> f = []() {});
	std::function<void()> mDoSomething;
	
};
Head::Head(sf::Vector2i pos, sf::Color color)
	: Segment(pos, color)
{
}
void Head::doSomething(std::function<void()> f)
{
}
class MapCell : UniversalDad
{
public:
	MapCell(sf::Color color = sf::Color::Cyan, Head* head = nullptr, bool mIsBuzy = false);

	virtual void doSomething(); //сделай что-то
	Head* mHead; //оно и не нужно наверно
	bool mIsBuzy;
};
MapCell::MapCell(sf::Color color, Head* head, bool mIsBuzy) :
	UniversalDad(color)
	, mHead(nullptr)
{
}
void MapCell::doSomething()
{
	std::cout << "Step" << std::endl;
	if (mIsBuzy)
		std::cout << "YOU Loose" << std::endl;
}

class Apple : public MapCell
{
public:
	Apple();
	~Apple();
	virtual void doSomething();
private:
};

Apple::Apple()
{
}

Apple::~Apple()
{
}

void Apple::doSomething()
{
	//mHead
}


void addSegment()
{

}
void moveSnake(sf::Keyboard::Key& key, std::vector<std::vector<MapCell>>& map, std::vector<Segment>& segments)
{
	sf::Vector2i oldPos = segments[0].mPos;
	if (key == sf::Keyboard::D)
		segments[0].mPos.x += (1);
	else if (key == sf::Keyboard::S)
		segments[0].mPos.y += (1);
	else if (key == sf::Keyboard::A)
		segments[0].mPos.x -= (1);
	else if (key == sf::Keyboard::W)
		segments[0].mPos.y -= (1);

	map[oldPos.y][oldPos.x].mIsBuzy = true;
	int k = segments.size() - 1;
	map[segments[k].mPos.y][segments[k].mPos.x].mIsBuzy = false;

	for (size_t i = 1; i < segments.size(); i++)
	{
		sf::Vector2i t = segments[i].mPos;
		segments[i].mPos = oldPos;
		oldPos = t;
	}


}
void drawFrame(sf::RenderWindow& window, std::vector<Segment>& segments, sf::CircleShape& shape)
{
	for (size_t i = 0; i < segments.size(); i++)
	{
		shape.setFillColor(segments[i].mColor);
		shape.setPosition(sf::Vector2f(segments[i].mPos.x * (shape.getRadius() * 2),
									   segments[i].mPos.y * (shape.getRadius() * 2)));
		window.draw(shape);
	}
}
int main()
{
	sf::RenderWindow window(sf::VideoMode(800, 800), "OneFileSFMLSnake");
	sf::CircleShape shape(25.f);

	//
	std::vector<std::vector<MapCell>> map;
	std::vector<MapCell> line;
	for (std::size_t j = 0; j < 25; j++) //захардкожено, потом подумаю, изменить ли или вообще через меню задать
		line.push_back((*new MapCell));
	for (std::size_t i = 0; i < 25; i++)
		map.push_back(line);
	line.~vector();
	//
	//
	std::vector<Segment> segments;
	segments.push_back((*new Head	(sf::Vector2i(1, 1), sf::Color::Red)));
	segments.push_back((*new Segment(sf::Vector2i(2, 1), sf::Color::White))); map[segments[1].mPos.y][segments[1].mPos.x].mIsBuzy = true;
	segments.push_back((*new Segment(sf::Vector2i(3, 1), sf::Color::White))); map[segments[2].mPos.y][segments[2].mPos.x].mIsBuzy = true;
	//
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.key.code == sf::Keyboard::Escape)
				window.close();

			if (event.type == sf::Event::KeyPressed)
			{
				moveSnake(event.key.code, map, segments);

				map[segments[0].mPos.y][segments[0].mPos.x].doSomething();
			}


		}



		window.clear();
		drawFrame(window, segments, shape);
		window.display();
	}

	return 0;
}

