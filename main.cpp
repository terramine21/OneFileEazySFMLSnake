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
		StandartMove,
		MoveUp,
		MoveDown,
		MoveLeft,
		MoveRight,
		GameOver,
	};

	enum Move
	{
		Up, Down, Left, Right
	};
}

template <typename Resource, typename Identifier>
class ResourceHolder
{
public:
	ResourceHolder() {}
	~ResourceHolder() {}

	void load(Identifier id, const std::string& filename)
	{
		std::unique_ptr<Resource> resource(new Resource);
		if (!resource->loadFromFile(filename))
			throw std::runtime_error("ResourceHolder::load - Failed to load " + filename);
		auto inserted = mTextureMap.insert(std::make_pair(id, std::move(resource))); //как бы создаётся пара из базовой пары и ответа, удалась ли вставка

		assert(inserted.second);
	}

	const Resource& get(Identifier id) const
	{
		auto found = mTextureMap.find(id);

		assert(found != mTextureMap.cend());

		return *found->second;
	}

private:
	std::map<Identifier, std::unique_ptr<Resource>> mTextureMap; //уникальный указатель, то есть указатель если два указателя указывает на одно, то остаётся только последне созданный указатель
};
typedef ResourceHolder<sf::Texture, Cells::ID> TextureHolder;

class Cell //знает куда, как и что рисовать
{
public:
	typedef std::unique_ptr<Cell> Ptr;
	
	struct Context
	{
		Context(sf::RenderWindow& window, float scale,  TextureHolder& textures)
			: window(&window)
			, scale(scale)
			, textures(&textures)
		{}
		Context() {}

		sf::RenderWindow* window;
		float scale;
		TextureHolder* textures;
	};

	Cell(sf::Vector2i pos, Cells::ID id, Context& context)
		: mContext(&context)
		, mID(id)
		, mShape(sf::Vector2f(1,1))
	{
		mShape.setScale(sf::Vector2f(mContext->scale, mContext->scale));
		mShape.setPosition(sf::Vector2f(pos.x * mContext->scale, pos.y * mContext->scale));
		mShape.setTexture(&mContext->textures->get(id));
	}
	void draw()
	{
		mContext->window->draw(mShape);
	}
	void reassignment(Cells::ID id) 
	{
		mID = id;
		mShape.setTexture(&mContext->textures->get(id));
	}

	 Cells::ID returnID() 
	{
		 return mID;
	} //это по сути айди действия, которое обрабатывается в MapManage

	 sf::Vector2i getPos()
	 {
		 return sf::Vector2i(mShape.getPosition().x / mShape.getScale().x, mShape.getPosition().y / mShape.getScale().y);
	 }
	 void setPos(sf::Vector2i pos)
	 {
		 mShape.setPosition(sf::Vector2f(pos.x * mShape.getScale().x, pos.y * mShape.getScale().y));
	 }

	Context* getContext() { return mContext; }
private:
	Context* mContext;
	Cells::ID mID;
	sf::RectangleShape mShape;
};

class MapManager
{
public:
	MapManager(Cell::Context& context)
		: mContext(&context)
	{
	}

	void draw(std::vector<Cell*> snake)
	{
		for (auto line : Map)
			for (auto cell : line)
				cell->draw();

		for (auto seg : snake)
			seg->draw();
	}

	void reassignment(sf::Vector2i pos, Cells::ID id) //переопределение
	{
		Map[pos.y][pos.x]->reassignment(id); 
	}

	Cell* getCell(sf::Vector2i pos)
	{
		return Map[pos.y][pos.x];
	}

	void create()
	{
		int size = 25;
		for (std::size_t i = 0; i < size; i++)
			for (std::size_t j = 0; j < size; j++)
			{
				Map.resize(size);
				if (i == 0 || i == (size - 1))
					Map[i].push_back(new Cell(sf::Vector2i(j, i), Cells::Wall,		*mContext));
				else if (j == 0 || j == (size - 1))
					Map[i].push_back(new Cell(sf::Vector2i(j, i), Cells::Wall,		*mContext));
				else
					Map[i].push_back(new Cell(sf::Vector2i(j, i), Cells::Standart,	*mContext));
			}
	}


private:

private:
	Cell::Context*						mContext;
	std::vector<std::vector<Cell*>>		Map;
};

class Snake
{
public:

private:

};

class Game //Application, потом когда буду добавлять меню изменю
{
public:
	Game()
		: mWindow(sf::VideoMode(800, 800), "OneFileSFMLSnake")
		, mTextures()
		, mContext(mWindow, 25, mTextures) //пока так
		, mapManager(mContext)
		, direct(Action::Right)
	{
		addTexture();
		mapManager.create();

		mSnake.push_back(new Cell(sf::Vector2i(2, 2), Cells::Wall, mContext));
	}

	void run()
	{

		while (mWindow.isOpen())
		{
			sf::Event event;
			while (mWindow.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					mWindow.close();
				if (event.key.code == sf::Keyboard::Escape)
					mWindow.close();
				if (event.key.code == sf::Mouse::Left)
					doSomething(Action::StandartMove);
			}

			mWindow.clear();
			mapManager.draw(mSnake);
			mWindow.display();
		}
	}

private:
	void cellEffect(Cells::ID id)
	{
		Action::ID a = Action::None;
		if (id == Cells::Standart)
			a = Action::None;
		else if (id == Cells::Wall)
			a = Action::GameOver;
		else if (id == Cells::Apple)
			a = Action::AddSegment;
		else if (id == Cells::SnakeSegment)
			a = Action::GameOver;

			doSomething(a);
	}
	void doSomething(Action::ID action)
	{
		switch (action)
		{
		case Action::		None:
			break;
		case Action::		AddSegment:
			//return [this]() {};
			break;
		case Action::		StandartMove:
		{
			sf::Vector2i oldPos = mSnake[0]->getPos();

			if (direct == Action::Right)
				mSnake[0]->setPos(sf::Vector2i(mSnake[0]->getPos().x + 1, mSnake[0]->getPos().y));
			else if (direct == Action::Down)
				mSnake[0]->setPos(sf::Vector2i(mSnake[0]->getPos().x, mSnake[0]->getPos().y + 1));
			else if (direct == Action::Left)
				mSnake[0]->setPos(sf::Vector2i(mSnake[0]->getPos().x - 1, mSnake[0]->getPos().y));
			else if (direct == Action::Up)
				mSnake[0]->setPos(sf::Vector2i(mSnake[0]->getPos().x, mSnake[0]->getPos().y - 1));

			int k = mSnake.size() - 1;
			for (size_t i = 1; i < mSnake.size(); i++)
			{
				sf::Vector2i t = mSnake[i]->getPos();
				mSnake[i]->getPos() = oldPos;
				oldPos = t;
			}
		}
			break;
		case Action::		MoveUp:
			//return [this]()
			break;
		case Action::		MoveDown:
			//return [this]() {};
			break;
		case Action::		MoveLeft:
			//return [this]() {};
			break;
		case Action::		MoveRight:
			//return [this]() {};
			break;
		case Action::		GameOver:
			//return [this]() {};
			break;
		default:
			break;
		}
	}
	void addTexture()
	{
		mTextures.load(Cells::Standart, "Textures/Standart.png"); //потом отдельно вынесу как скрины кадра, если не будет лень
		mTextures.load(Cells::Wall, "Textures/Wall.png");
		mTextures.load(Cells::Apple, "Textures/Apple.png");
	}
	typedef void воид;
	воид кек()
	{
		std::cout << "kek" << std::endl;
	}
private:
	MapManager mapManager; //в нём же и карта
	// избавиться от следующего
	std::vector<Cell*> mSnake;
	Action::Move direct;
	//
	sf::RenderWindow mWindow;
	TextureHolder mTextures;
	Cell::Context mContext;
};

int main()
{
	Game game;
	game.run();
	

	return 0;
}
