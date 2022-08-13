#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <vector>
#include <functional>
#include <iostream>
#include <cassert>

const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);

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
		TitleScreen,
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
namespace States
{
	enum ID
	{
		None,
		Game, 
		Menu,
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

class StateStack; //для StateStack* mStack;
class State
{
public:
	typedef std::unique_ptr<State> Ptr;

public:
				 State(StateStack& stack, Cell::Context context)
		: mStack(&stack)
		, mContext(context)
	{
	}
				 //State();
	virtual		 ~State()
	{
	}
	virtual void draw() = 0;
	virtual bool update(sf::Time dt) = 0;
	virtual bool handleEvent(const sf::Event& event) = 0;

	template<typename SomethingSF>
	void centerOrigin(SomethingSF& somethingSF)
	{
		sf::FloatRect bounds = somethingSF.getLocalBounds();
		somethingSF.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
	}

protected:
	void			requestStackPush(States::ID stateID);
	void			requestStackPop();
	void			requestStateClear();
	Cell::Context	getContext() const
	{
		return mContext;
	}
private:
	StateStack*			mStack;
	Cell::Context		mContext;
};
class StateStack : private sf::NonCopyable
{
public:
	enum Action
	{
		Push,
		Pop,
		Clear,
	};
public:
	explicit StateStack(Cell::Context context)
		: mStack()
		, mPendingList()
		, mContext(context)
		, mFactories()
	{
	}
	explicit StateStack()
	{
	}

	template <typename T>
	void registerState(States::ID stateID)
	{
		mFactories[stateID] = [this]() {return State::Ptr(new T(*this, mContext)); };
	}

	void update(sf::Time dt)
	{
		for (auto itr = mStack.rbegin(); itr != mStack.rend(); ++itr)
		{
			if (!(*itr)->update(dt))
				break;
		}
		applyPendingChanges();
	}
	void draw()
	{
		for (auto& state : mStack)
		{
			state->draw();
		}
	}

	void handleEvent(const sf::Event& event)
	{
		for (auto itr = mStack.rbegin(); itr != mStack.rend(); ++itr)
		{
			if (!(*itr)->handleEvent(event)) 
				return;
		}
		applyPendingChanges();
	}

	void pushState(States::ID stateID)
	{
		mPendingList.push_back(PendingChange(Push, stateID));
	}
	void popState()
	{
		mPendingList.push_back(PendingChange(Pop));
	}
	void clearStates()
	{
		mPendingList.push_back(PendingChange(Clear));
	}
	bool isEmpty() const
	{
		return mStack.empty();
	}

private:
	State::Ptr createState(States::ID stateID)
	{
		auto found = mFactories.find(stateID);
		assert(found != mFactories.end());
		return found->second();
	}
	void applyPendingChanges()
	{
		for (PendingChange change : mPendingList)
		{
			switch (change.action)
			{
			case Push:
				mStack.push_back(createState(change.stateID));
				break;
			case Pop:
				mStack.pop_back();
				break;
			case Clear:
				mStack.clear();
				break;
			default:
				break;
			}
			mPendingList.clear();
		}
	}
private:
	struct PendingChange
	{
		explicit PendingChange(Action action, States::ID stateID = States::None)
			: action(action)
			, stateID(stateID)
		{
		}
		Action action;
		States::ID stateID;
	};
private:
	std::vector<State::Ptr> mStack;
	std::vector<PendingChange> mPendingList;
	Cell::Context mContext;
	std::map<States::ID, std::function<State::Ptr()>> mFactories;
};
void			State::requestStackPush(States::ID stateID)
{
	mStack->pushState(stateID);
}
void			State::requestStackPop()
{
	mStack->popState();
}
void			State::requestStateClear()
{
	mStack->clearStates();
}


//class GameState : public State
//{
//public:
//	GameState(StateStack& stack, Cell::Context context): State(stack, context){}
//	virtual void draw(){}
//	virtual bool update(sf::Time dt) { return true; }
//	virtual bool handleEvent(const sf::Event& event){ return true; }
//
//private:/*
//	World					mWorld;
//	Player					mPlayer;*/
//};

class MenuState : public State
{
public:
	MenuState(StateStack& stack, Cell::Context context)
		: State(stack, context)
		, mOptions()
		, mOptionIndex(0)
	{
		sf::Texture texture = context.textures->get(Cells::TitleScreen);
		//sf::Font font = context.fonts->get(Fonts::Main);

		sf::Text playOption;
		//playOption.setFont(font);
		playOption.setString("Play");
		centerOrigin(playOption);
		sf::Vector2f pos = context.window->getView().getSize() / 2.f;
		playOption.setPosition(pos);
		mOptions.push_back(playOption);
		mOptions[0].setFillColor(sf::Color::Red);

		playOption.setString("Exit");
		centerOrigin(playOption);
		pos.y += (playOption.getLocalBounds().height + 2.f);
		playOption.setPosition(pos);
		mOptions.push_back(playOption);

		playOption.setString("BOOK, PLEASE, STOP!");
		centerOrigin(playOption);
		pos.y += (playOption.getLocalBounds().height + 2.f);
		playOption.setPosition(pos);
		mOptions.push_back(playOption);
	}

	virtual void draw()
	{
		sf::RenderWindow& window = *getContext().window;

		window.setView(window.getDefaultView());
		window.draw(mBackgroundSprite);

		for (auto& text : mOptions)
			window.draw(text);
	}
	virtual bool update(sf::Time dt)
	{
		return true;
	}
	virtual bool handleEvent(const sf::Event& event)
	{
		if (event.type != sf::Event::KeyPressed)
			return false;

		if (event.key.code == sf::Keyboard::Up)
		{
			if (mOptionIndex > 0)
				mOptionIndex--;
			else
				mOptionIndex = mOptions.size() - 1;
			updateOptionText();
		}
		else if (event.key.code == sf::Keyboard::Down)
		{
			if (mOptionIndex < mOptions.size() - 1)
				mOptionIndex++;
			else
				mOptionIndex = 0;
		}
		updateOptionText();

		if (event.key.code == sf::Keyboard::Enter)
			if (mOptionIndex == Play) //Play в enum, то есть число, а значит индекс и Play можно сравнить
			{
				requestStackPop();
				requestStackPush(States::Game);
			}
			else if (mOptionIndex == Exit)
			{
				requestStackPop(); //так как в стэке не будет никаких состояний. то и игра закроется ибо это проверят в Application в run()
			}
			else if (mOptionIndex == BOOK_PLEASE_STOP)
			{
				std::cout << "Book, please, STOP!" << std::endl;
			}
	}

	void updateOptionText()
	{
		if (mOptions.empty())
			return;

		for (sf::Text& text : mOptions)
			text.setFillColor(sf::Color::White);

		mOptions[mOptionIndex].setFillColor(sf::Color::Red);
	}

private:
	enum OptionNames
	{
		Play, Exit, BOOK_PLEASE_STOP
	};
	sf::Sprite mBackgroundSprite;
	std::vector<sf::Text> mOptions;
	std::size_t mOptionIndex;
};



class Game //Application, потом когда буду добавлять меню изменю
{
public:
	Game()
		: mWindow(sf::VideoMode(800, 800), "OneFileSFMLSnake")
		, mTextures()
		, mContext(mWindow, 25, mTextures, mSound) //пока так
		, mStateStack(mContext)
		, mapManager(mContext)
		, direct(Action::Right)
		, mSpeed(0.5)
	{
		addTexture();
		mapManager.create();
		doSomething(Action::SpawnSnake);
		doSomething(Action::SpawnApple);

		mTextures.load(Cells::TitleScreen, "Textures/TitleScreen.png");

		registerStates(); //регистрация состояний, очевидно жи, кхем
		mStateStack.pushState(States::Menu);
	}

	void run()
	{
		sf::Clock clock;
		sf::Time timeSinceLastUpdate = sf::Time::Zero;

		while (mWindow.isOpen())
		{
			sf::Time dt = clock.restart();
			timeSinceLastUpdate += dt;
			while (timeSinceLastUpdate > TimePerFrame)
			{
				timeSinceLastUpdate -= TimePerFrame;

				progressInput();
				update(TimePerFrame);

				if (mStateStack.isEmpty())
					mWindow.close();
			}
			
			render();
		}
	}

	void update(sf::Time dt)
	{
		mStateStack.update(dt);
	}

	//bool update(sf::Event event, sf::Clock& clock)
	//{

	//		if (event.key.code == sf::Keyboard::A && direct != Action::Right)
	//			direct = Action::Left;
	//		else if (event.key.code == sf::Keyboard::D && direct != Action::Left)
	//			direct = Action::Right;
	//		else if (event.key.code == sf::Keyboard::W && direct != Action::Down)
	//			direct = Action::Up;
	//		else if (event.key.code == sf::Keyboard::S && direct != Action::Up)
	//			direct = Action::Down;
	//	if (clock.getElapsedTime().asSeconds() >= mSpeed)
	//	{
	//		doSomething(Action::StandartMove);
	//		clock.restart();
	//	}
	//	return true;
	//}
	void draw()
	{
		mapManager.draw(mSnake);
	}
private:
	void progressInput()
	{
		sf::Event event;
		while (mWindow.pollEvent(event))
		{
			mStateStack.handleEvent(event);

			if (event.type == sf::Event::Closed)
				mWindow.close();
			else if (event.key.code == sf::Keyboard::Tilde)
				mWindow.close();
		}
	}
	void registerStates()
	{
		mStateStack.registerState<MenuState>(States::Menu);
		//mStateStack.registerState<GameState>(States::Game);
	}
	//void render()
	//{
	//	mWindow.clear();
	//	draw();
	//	mWindow.display();
	//}

	void render()
	{
		mWindow.clear();
		mStateStack.draw();
		mWindow.setView(mWindow.getDefaultView());
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
	StateStack	mStateStack;
	std::map<Audios::ID, sf::Sound> shortSound;
	//
	float mSpeed;
	//
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
