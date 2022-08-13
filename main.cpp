#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <vector>
#include <functional>
#include <iostream>
#include <cassert>


namespace Cells
{
	enum ID
	{
		None,
		Standart,
		Wall,
		Apple,
		SnakeSegment,
		SnakeHead,
	};
}
namespace Audios
{
	enum ID
	{
		eat,
	};
}
namespace Action
{
	enum ID
	{
		None,
		SpawnSnake,
		AddSegment,
		SpawnApple,
		//DeledeApple,
		MoveApple,
		StandartMove,
		MoveUp,
		MoveDown,
		MoveLeft,
		MoveRight,
		CheckCellAction,
		GameOver,
		MoveSnake,
		PlayEat,
	};

	enum Move
	{
		Up, Down, Left, Right, Wait
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
typedef ResourceHolder<sf::SoundBuffer, Audios::ID> SoundHolder;

class Cell //знает куда, как и что рисовать
{
public:
	typedef std::unique_ptr<Cell> Ptr;
	
	struct Context
	{
		Context(sf::RenderWindow& window, float scale,  TextureHolder& textures, SoundHolder& sound)
			: window(&window)
			, scale(scale)
			, textures(&textures)
			, sound(&sound)
		{}
		Context() {}

		sf::RenderWindow* window;
		float scale;
		TextureHolder* textures;
		SoundHolder* sound;
	};

	Cell(sf::Vector2i pos, Cells::ID id, Context& context)
		: mContext(&context)
		, mID(id)
		, mShape(sf::Vector2f(1, 1))
		, addOn(nullptr)
	{
		mShape.setScale(sf::Vector2f(mContext->scale, mContext->scale));
		mShape.setPosition(sf::Vector2f(pos.x * mContext->scale, pos.y * mContext->scale));
		mShape.setTexture(&mContext->textures->get(id));
	}
	void draw()
	{
		mContext->window->draw(mShape);
		if(addOn)
			addOn->draw();
	}
	void reassignment(Cells::ID id) 
	{
		mID = id;
		mShape.setTexture(&mContext->textures->get(id));
	}

	Cells::ID getID()
	{
		if (mID == Cells::Standart && addOn)
			return addOn->getID();
		else
			return mID;
	}

	 sf::Vector2i getPos()
	 {
		 return sf::Vector2i(mShape.getPosition().x / mShape.getScale().x, mShape.getPosition().y / mShape.getScale().y);
	 }
	 void setPos(sf::Vector2i pos)
	 {
		 mShape.setPosition(sf::Vector2f(pos.x * mShape.getScale().x, pos.y * mShape.getScale().y));
	 }

	Context* getContext() 
	{
		return mContext;
	}

	void setAddOn(Cell* cell) 
	{
		addOn = cell; 
	}
private:
	Context* mContext;
	sf::RectangleShape mShape;
	Cells::ID mID;
	Cell* addOn; //надстройка
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
	void SetAddOn(sf::Vector2i pos, Cell* cell)
	{
		Map[pos.y][pos.x]->setAddOn(cell);
	}
	Cell* getCell(sf::Vector2i pos)
	{
		return Map[pos.y][pos.x];
	}
	std::size_t getSize() { return Map.size(); }

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


class Game //Application, потом когда буду добавлять меню изменю
{
public:
	Game()
		: mWindow(sf::VideoMode(800, 800), "OneFileSFMLSnake")
		, mTextures()
		, mContext(mWindow, 25, mTextures, mSound) //пока так
		, mapManager(mContext)
		, direct(Action::Right)
	{
		addTexture();
		mapManager.create();
		doSomething(Action::SpawnSnake);
		doSomething(Action::SpawnApple);
	}

	void run()
	{
		sf::Clock clock;
		float speed = 0.5;



		render();

		while (mWindow.isOpen())
		{
			sf::Event event;
			while (mWindow.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					mWindow.close();
				if (event.key.code == sf::Keyboard::Escape)
					mWindow.close();
			}

			if (clock.getElapsedTime().asSeconds() >= speed)
			{
				if (event.key.code == sf::Keyboard::A && direct != Action::Right)
					direct = Action::Left;
				else if (event.key.code == sf::Keyboard::D && direct != Action::Left)
					direct = Action::Right;
				else if (event.key.code == sf::Keyboard::W && direct != Action::Down)
					direct = Action::Up;
				else if (event.key.code == sf::Keyboard::S && direct != Action::Up)
					direct = Action::Down;


				update();
				render();

				clock.restart();
			}
		}
	}



	void update()
	{
		doSomething(Action::StandartMove);
	}
	void draw()
	{
		mapManager.draw(mSnake);
	}
private:




	void render()
	{
		mWindow.clear();
		draw();
		mWindow.display();
	}


	void cellEffect(Cell* cell)
	{
		Cells::ID id = cell->getID();

		if (id == Cells::Standart)
			doSomething(Action::None);
		else if (id == Cells::Wall)
			doSomething(Action::GameOver);
		else if (id == Cells::Apple)
		{
			doSomething(Action::AddSegment);
			doSomething(Action::MoveApple);
			doSomething(Action::PlayEat);
		}
		else if (id == Cells::SnakeSegment)
			doSomething(Action::GameOver);


	}
	void doSomething(Action::ID action)
	{
		switch (action)
		{
		case Action::		None:
			break;
		case Action::		AddSegment:
			mSnake.push_back(new Cell(oldPos, Cells::SnakeSegment, mContext));
			break;
		case Action::		StandartMove:
		{
			sf::Vector2i t(0, 0);
			if (direct == Action::Right)
				t.x += 1;
			else if (direct == Action::Down)
				t.y += 1;
			else if (direct == Action::Left)
				t.x -= 1;
			else if (direct == Action::Up)
				t.y -= 1;
			moveSnake(t);
		}
			break;
		case Action::		GameOver:
		{
			std::cout << "YOU LOSE" << std::endl; //временно
			exit(25);
		}
		break;
		case Action::		SpawnApple:
		{
			std::srand(time(0));
			mApple = new Cell(
				sf::Vector2i(1 + rand() % (mapManager.getSize() - 2), 1 + rand() % (mapManager.getSize() - 2)),
				Cells::Apple, mContext);
			mapManager.SetAddOn(mApple->getPos(), mApple);
		}
		break;
		case Action::		MoveApple:
		{
			std::srand(time(0));
			sf::Vector2i apos;
			
			while (true)
			{
				apos = sf::Vector2i(rand() % (mapManager.getSize()), rand() % (mapManager.getSize()));
				if (mapManager.getCell(apos)->getID() == Cells::Standart)
				{
					mApple->setPos(apos);
					mapManager.SetAddOn(mApple->getPos(), mApple);
					break;
				}

			}

		}
		break;
		case Action::		SpawnSnake:
		{
			mSnake.push_back(new Cell(sf::Vector2i(2, 2), Cells::SnakeHead, mContext));
			mSnake.push_back(new Cell(sf::Vector2i(2, 2), Cells::SnakeSegment, mContext));
			mSnake.push_back(new Cell(sf::Vector2i(2, 2), Cells::SnakeSegment, mContext));
		}
		break;
		case Action::		PlayEat:
		{
			shortSound[Audios::eat].play();
		}
		default:
			break;
		}
	}
	void addTexture()
	{
		mTextures.load(Cells::Standart, "Textures/Standart.png"); //потом отдельно вынесу как скрины кадра, если не будет лень
		mTextures.load(Cells::Wall, "Textures/Wall.png");
		mTextures.load(Cells::Apple, "Textures/Apple.png");
		mTextures.load(Cells::SnakeHead, "Textures/SnakeHead.png");
		mTextures.load(Cells::SnakeSegment, "Textures/SnakeSegment.png");
		//
		mSound.load(Audios::eat, "Textures/hrum.wav");
		sf::Sound s;
		s.setBuffer(mContext.sound->get(Audios::eat));
		s.setVolume(100);
		shortSound[Audios::eat] = s;
		//
	}
	typedef void воид;
	воид кек()
	{
		std::cout << "kek" << std::endl;
	}

	void moveSnake(sf::Vector2i move)
	{
		oldPos = mSnake[0]->getPos();
		mSnake[0]->setPos(sf::Vector2i(mSnake[0]->getPos() + move));

		for (size_t i = 1; i < mSnake.size(); i++)
		{
			sf::Vector2i t = mSnake[i]->getPos();
			mapManager.SetAddOn(oldPos, mSnake[i]);
			mSnake[i]->setPos(oldPos);
			oldPos = t;
		}
		mapManager.SetAddOn(oldPos, nullptr);

		cellEffect(mapManager.getCell(mSnake[0]->getPos()));
		mapManager.SetAddOn(mSnake[0]->getPos(), mSnake[0]);
	}

private:
	MapManager mapManager; //в нём же и карта
	// избавиться от следующего
	std::vector<Cell*> mSnake;
	Action::Move direct;
	sf::Vector2i oldPos;
	Cell* mApple;
	//
	sf::RenderWindow mWindow;
	TextureHolder mTextures;
	SoundHolder mSound;
	Cell::Context mContext;
	std::map<Audios::ID, sf::Sound> shortSound;
};

int main()
{
	try
	{
		Game game;
		game.run();
	}
	catch (const std::exception& e)
	{
		std::cout << "\nEXCEPTION: " << e.what() << std::endl;
	}


	return 0;
}
