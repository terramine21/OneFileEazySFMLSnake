#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <iostream>
#include <cassert>

//есть две карты, одна карта - массив с яблоками, змейкой и прочим на доске
//вторая - прост заполнение пустоты и рисование кадра я вижу следующим образом:
/*
	void World::FrameMaker(std::size_t mapSize, vector<Snake>& snake)
	{
		vector<vector<Cells::ID>> background = ......
		...
	}
	void World::FrameMaker(vector<vector<Cells::ID>>& frame)
	{
	}

*/
//или пойти по-другому и дать классу знать только свою позицию, но тогда придётся проверять все массивы, и наложение кря..

//Сейчас я хочу пойти по пути, когда я нахожу в стэке клеток клетку и происходит событие и всё через карту.
//Первоначальная отрисовка кадра - рисуется всё, а потом только те места, которые изменились.
//Хвост змейки заменятеся клеткой, кушаем яблоко и не заменятеся, по сути отказываюсь от вектора змейки,
//а голова единственная имеет направление, которое передаётся в функцию передвижения.
//У каждой клетки есть своё действие, которое активируется при наступлении на неё головой.
//Cell - просто шалон класса, по которому строится уже всё остальное
//CellStak - хранит клетки
//

//namespace Cells
//{
//	enum ID
//	{
//		Standart, 
//		Wall,
//		Apple,
//		SnakeSegment,
//		SnakeHead,
//	};
//}
//
//class CellStack;
//class Cell
//{
//public:
//	typedef std::unique_ptr<Cell> Ptr;
//	Cell(CellStack& stack, sf::RenderWindow& window)
//		: mStack(&stack)
//		, mWindow(&window)
//	{
//	}
//
//	//virtual void draw() = 0;
//	//virtual bool update() = 0;
//	//virtual bool update(sf::Time dt) = 0;
//	////virtual bool handleEvent(const sf::Event& event) = 0;
//	//virtual bool doWork() = 0;
//
//private:
//	CellStack*		  mStack;
//	sf::RenderWindow* mWindow;
//};
//
//class CellStack
//{
//public:
//	CellStack(sf::RenderWindow& window) : mStack(), mWindow(&window)
//	{
//
//	}
//
//	template <typename T>
//	void registerCell(Cells::ID id)
//	{
//		mStack[id] = std::make_unique<T>(*this, *mWindow);
//	}
//private:
//	std::map<Cells::ID ,Cell::Ptr> mStack;
//	sf::RenderWindow* mWindow;
//};
//
//class SnakeHead : public Cell
//{
//public:
//	SnakeHead(CellStack& stack, sf::RenderWindow& window)
//		: Cell(stack, window)
//	{
//	}
//private:
//
//};



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

	Cells::ID Map[20][20]
	{
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart},
		{Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart,Cells::Standart}
	};
}

class Cell
{
public:
	typedef std::unique_ptr<Cell> Ptr;

	Cell(sf::RenderWindow& window, sf::Color color = sf::Color::Black)
		: mWindow(window)
		, mColor(color)
	{}
	virtual void draw(sf::Vector2i pos, sf::CircleShape& shape) {}
	virtual void doSomething() { std::cout << "nothing" << std::endl; }

	sf::RenderWindow& getWindow() { return mWindow; }

	sf::Color mColor;
private:
	sf::RenderWindow& mWindow;

};

class Standart : public Cell
{
public:
	Standart(sf::RenderWindow& window, sf::Color color = sf::Color::Blue)
		: Cell(window, color)
	{

	}
		void draw(sf::Vector2i pos, sf::CircleShape & shape)
		{
			shape.setPosition(pos.x*shape.getRadius()*2, pos.y*shape.getRadius() * 2);
			shape.setFillColor(mColor);
			getWindow().draw(shape);
		}
		void doSomething() 
		{
			std::cout << "Standart" << std::endl; 
		}
};

class MapManage
{
public:
	MapManage(sf::RenderWindow& window, sf::CircleShape& shape) 
		: mWindow(window)
		, mShape(shape)
	{}

	void firstDrawMap(sf::CircleShape shape)
	{
		for (std::size_t i = 0; i < 20; i++)
			for (std::size_t j = 0; j < 20; j++)
				doSomething(sf::Vector2i(j, i), Cells::Map[j][i]);
	}


	void doSomething(sf::Vector2i pos, Cells::ID id) 
	{ 
		mVector[id]->draw(pos, mShape);
		mVector[id]->doSomething(); 
	}

	template <typename T>
	void registerCell(Cells::ID id)
	{
		mVector[id] = std::make_unique<T>(mWindow);
	}

private:
	std::map < Cells::ID , Cell::Ptr > mVector;
	sf::RenderWindow& mWindow;
	sf::CircleShape& mShape;
};
int main()
{
	sf::RenderWindow window(sf::VideoMode(800, 800), "OneFileSFMLSnake");

	sf::CircleShape shape(25.f);
	shape.setFillColor(sf::Color::Red);

	MapManage vec(window, shape);
	vec.registerCell<Standart>(Cells::Standart);


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

		vec.firstDrawMap(shape);

		//window.draw(shape);
		window.display();
	}

	return 0;
}
