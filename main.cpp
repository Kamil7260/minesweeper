#include <SFML/Graphics.hpp>
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <memory>

class AActor;

class CGame;
template<class Obj>
class CLevel;

class CNumber;

class CBlock;

class CMineBlock;

class CSaveBlock;

class CMap;

enum Status
{
    game = 0,
    close = 1,
    lose=2,
    menu=3
};

enum BlockStatus
{
    hide = 0,
    show = 1,
    set = 2
};
///////////////////////////////////////////////////////////////////////////////////////
class AActor
{
public:
    virtual void Move(const sf::Vector2f delta)
    {
        m_position+=delta;
    }

    virtual void SetPosition(const sf::Vector2f pos)
    {
        m_position = pos;
    }

    virtual void Update(const float &deltaTime) = 0;

    virtual sf::Vector2f GetPosition()
    {
        return m_position;
    }

    virtual void Draw() = 0;

private:
    sf::Vector2f m_position;
};


///////////////////////////////////////////////////////////////////////////////////////
class CGame
{
private:
    CGame();
    CGame(const CGame & );
public:
    static CGame &Instance()
    {
        static CGame game;
        return game;
    }
    void Run();
    void Menu();
    void Stage();

    void setStatus(Status s)
    {
        status = s;
    }
    Status getStatus()
    {
        return status;
    }
    std::shared_ptr<sf::RenderWindow> getWindow()
    {
        return window;
    }
private:
    std::shared_ptr<sf::RenderWindow> window;
    Status status;

};

///////////////////////////////////////////////////////
template<class Obj>
class CLevel
    :public AActor
{
public:
    void Draw()
    {
        for(auto i = 0u; i<TObjects.size(); i++)
        {
            TObjects[i]->Draw();
        }
    }
    void Update(const float &deltaTime)
    {
        for(auto i = 0u; i<TObjects.size(); i++)
        {
            TObjects[i]->Update(deltaTime);
        }
    }
    void Add(std::shared_ptr<Obj> actor)
    {
        if(actor) TObjects.push_back(actor);
    }
    size_t getSize()
    {
        return TObjects.size();
    }
    template<class Object>
    friend void GenerateNumber(CLevel<Object> &Level);

    template<class Object>
    friend bool CheckStatus(CLevel<Object> &TTObjects,BlockStatus status);

    template<class Object>
    friend bool LookForStatus(CLevel<Object> &Level,BlockStatus status);

    CLevel() {}
    ~CLevel() {}
private:
    std::vector<std::shared_ptr<Obj> > TObjects;
};
///////////////////////////////////////////////////////////////////////////////////////
template<class Object>
void GenerateNumber(CLevel<Object> &Level)
{
    for(auto i = 0u; i<Level.TObjects.size(); i++)
        {
            Level.TObjects[i]->GenerateConstNumber();
        }
}
///////////////////////////////////////////////////////////////////////////////////////
template<class Object>
bool CheckStatus(CLevel<Object> &Level,BlockStatus status)
{
    for(auto i = 0u; i<Level.TObjects.size(); i++)
    {
        if(Level.TObjects[i]->GetStatus()!=status)
            return false;
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////
template<class Object>
bool LookForStatus(CLevel<Object> &Level,BlockStatus status)
{
    for(auto i = 0u; i<Level.TObjects.size(); i++)
    {
        if(Level.TObjects[i]->GetStatus()==status)
            return true;
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////////////
class CBlock
    :public AActor
{
public:
    virtual bool IsPressed();
    virtual void SetBomb(bool s)
    {
        isBomb = s;
    }
    virtual bool IsBomb()
    {
        return isBomb;
    }
    virtual BlockStatus GetStatus()
    {
        return status;
    }
    virtual void SetStatus(BlockStatus s)
    {
        status =s;
    }
    virtual void SetPosition(const sf::Vector2f pos)
    {
        AActor::SetPosition(pos);
        shape.setPosition(pos);
    }
    virtual void SetSize(const sf::Vector2f s)
    {
        size = s;
        shape.setSize(size);
    }
    sf::Vector2f GetSize()
    {
        return size;
    }
    virtual void SetColor(sf::Color color)
    {
        shape.setFillColor(color);
    }
    float lastFrame;
    virtual void GenerateConstNumber() {}
    virtual void ImproveNumber() {}
    virtual void Draw()
    {
        CGame::Instance().getWindow()->draw(shape);
    }
private:
    bool isBomb;
    sf::Vector2f size;
    BlockStatus status;
    sf::RectangleShape shape;

};
///////////////////////////////////////////////////////////////////////////////////////
class CNumber
{
public:
    CNumber(unsigned int i,sf::Vector2f pos);
    ~CNumber() {}
    void Draw();

private:
    std::unique_ptr<sf::VertexArray> m_array;
};
///////////////////////////////////////////////////////////////////////////////////////
class CMineBlock
    :public CBlock
{
public:
    CMineBlock(sf::Vector2f pos = sf::Vector2f(0,0),sf::Vector2f Lsize = sf::Vector2f(40,40))
    {
        CBlock::SetColor(sf::Color::Blue);
        CBlock::SetPosition(pos);
        CBlock::SetSize(Lsize);
        CBlock::SetStatus(BlockStatus::hide);
        lastFrame = 0;
        CBlock::SetBomb(true);
    }
    void Update(const float &deltaTime);
};
///////////////////////////////////////////////////////////////////////////////////////
class CSaveBlock
    :public CBlock
{
public:
    CSaveBlock(sf::Vector2f pos = sf::Vector2f(0,0),sf::Vector2f Lsize = sf::Vector2f(40,40))
    {
        CBlock::SetColor(sf::Color::Blue);
        CBlock::SetPosition(pos);
        CBlock::SetSize(Lsize);
        CBlock::SetStatus(BlockStatus::hide);
        lastFrame = 0;
        CBlock::SetBomb(false);
        currentNumber = 0;
    }
    void GenerateConstNumber()
    {
        if(currentNumber!=0)
        {
            sf::Vector2f pos = AActor::GetPosition();
            pos.x+=5;
            pos.y+=5;
            m_number = std::make_unique<CNumber>(currentNumber,pos);
        }
    }
    void Draw()
    {
        CBlock::Draw();
        if(m_number && CBlock::GetStatus() == BlockStatus::show) m_number->Draw();
    }
    void ImproveNumber()
    {
        currentNumber++;
    }
    void Update(const float &deltaTime);
private:
    std::unique_ptr<CNumber> m_number;
    unsigned int currentNumber;

};
///////////////////////////////////////////////////////////////////////////////////////
class CMap
    :public AActor
{
public:
    bool IsWin()
    {
        if(CheckStatus(*m_mines,BlockStatus::set))
        {
            if(!LookForStatus(*m_saveBlocks,BlockStatus::set))
            {
                return true;
            }
        }
        return false;
    }
    CMap()
    {
        m_mines = std::make_shared<CLevel<CMineBlock> >();
        m_saveBlocks = std::make_shared<CLevel<CSaveBlock> >();
    }
    ~CMap() {}
    void Draw()
    {
        m_mines->Draw();
        m_saveBlocks->Draw();
    }
    void Generate(sf::Vector2f pos);
    void Update(const float &deltaTime)
    {
        if(CGame::Instance().getStatus()==Status::lose)
        {
            this->Open();
        }
        m_mines->Update(deltaTime);
        m_saveBlocks->Update(deltaTime);
    }
    void Open()
    {
        for(auto i = 0u; i<TBlocks.size(); i++)
        {
            TBlocks[i]->SetStatus(BlockStatus::show);
        }
    }
    void Happy();
private:
    std::vector<std::shared_ptr<CBlock>> TBlocks;
    int random[10];
    std::shared_ptr<CLevel<CMineBlock> > m_mines;
    std::shared_ptr<CLevel<CSaveBlock> > m_saveBlocks;

};
//////////////////////////////////////////////////////////////////////////////
CGame::CGame()
{
    status = Status::game;
    window = std::make_shared<sf::RenderWindow>(sf::VideoMode(800,600),"title");
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CGame::Menu()
{
    status = Status::game;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CGame::Stage()
{
    float deltaTime=1/60.f,frameStartTime;
    sf::Clock clock;

    CMap map;
    map.Generate(sf::Vector2f(50,50));

    while(status==Status::game)
    {

        frameStartTime = clock.getElapsedTime().asSeconds();
        sf::Event event;
        while(window->pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                status = Status::close;
        }
        window->clear();
        map.Draw();
        map.Update(deltaTime);

        if(status==Status::lose)
        {
            map.Open();
            map.Update(0);
            sf::sleep(sf::milliseconds(200));
            while(!sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                sf::Event event;
                while(window->pollEvent(event))
                {
                    if(event.type==sf::Event::Closed)
                    {
                        status = Status::close;
                        return;
                    }
                    window->clear();

                    map.Draw();

                    window->display();

                }
            }
            status = Status::menu;
        }
        if(map.IsWin())
        {
            sf::Clock clock;
            while(!sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                sf::Event event;
                while(window->pollEvent(event))
                {
                    if(event.type==sf::Event::Closed)
                    {
                        status = Status::close;
                        return;
                    }
                }
                window->clear();

                map.Draw();

                window->display();
                if(clock.getElapsedTime().asMilliseconds()>300)
                {
                    map.Happy();
                }
            }

            status = Status::menu;
        }
        window->display();

        deltaTime = clock.getElapsedTime().asSeconds() - frameStartTime;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CGame::Run()
{
    while(status!=Status::close)
    {
        if(status == Status::game)
        {
            Stage();
        }
        if(status == Status::menu)
        {
            Menu();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
CNumber::CNumber(unsigned int i,sf::Vector2f pos)
{
    if(i==1)
    {
        m_array = std::make_unique<sf::VertexArray>(sf::LinesStrip,3);
        m_array->operator[](0).position = sf::Vector2f(pos.x+5,pos.y+10);
        m_array->operator[](1).position = sf::Vector2f(pos.x+15,pos.y);
        m_array->operator[](2).position = sf::Vector2f(pos.x+15,pos.y+20);
        m_array->operator[](0).color = sf::Color(255, 153, 0);
        m_array->operator[](1).color = sf::Color(255, 153, 0);
        m_array->operator[](2).color = sf::Color(255, 153, 0);
    }
    else if(i==2)
    {
        m_array = std::make_unique<sf::VertexArray>(sf::LinesStrip,6);
        m_array->operator[](0).position = sf::Vector2f(pos.x+5,pos.y);
        m_array->operator[](1).position = sf::Vector2f(pos.x+15,pos.y);
        m_array->operator[](2).position = sf::Vector2f(pos.x+15,pos.y+5);
        m_array->operator[](3).position = sf::Vector2f(pos.x+5,pos.y+5);
        m_array->operator[](4).position = sf::Vector2f(pos.x+5,pos.y+10);
        m_array->operator[](5).position = sf::Vector2f(pos.x+15,pos.y+10);
        m_array->operator[](0).color = sf::Color(255, 153, 0);
        m_array->operator[](1).color = sf::Color(255, 153, 0);
        m_array->operator[](2).color = sf::Color(255, 153, 0);
        m_array->operator[](3).color = sf::Color(255, 153, 0);
        m_array->operator[](4).color = sf::Color(255, 153, 0);
        m_array->operator[](5).color = sf::Color(255, 153, 0);

    }
    else if(i==3)
    {
        m_array = std::make_unique<sf::VertexArray>(sf::LinesStrip,7);
        m_array->operator[](0).position = sf::Vector2f(pos.x+5,pos.y);
        m_array->operator[](1).position = sf::Vector2f(pos.x+15,pos.y);
        m_array->operator[](2).position = sf::Vector2f(pos.x+15,pos.y+5);
        m_array->operator[](3).position = sf::Vector2f(pos.x+5,pos.y+5);
        m_array->operator[](4).position = sf::Vector2f(pos.x+15,pos.y+5);
        m_array->operator[](5).position = sf::Vector2f(pos.x+15,pos.y+10);
        m_array->operator[](6).position = sf::Vector2f(pos.x+5,pos.y+10);
        m_array->operator[](0).color = sf::Color(255, 153, 0);
        m_array->operator[](1).color = sf::Color(255, 153, 0);
        m_array->operator[](2).color = sf::Color(255, 153, 0);
        m_array->operator[](3).color = sf::Color(255, 153, 0);
        m_array->operator[](4).color = sf::Color(255, 153, 0);
        m_array->operator[](5).color = sf::Color(255, 153, 0);
        m_array->operator[](6).color = sf::Color(255, 153, 0);
    }
    else if(i==4)
    {
        m_array = std::make_unique<sf::VertexArray>(sf::LinesStrip,4);
        m_array->operator[](0).position = sf::Vector2f(pos.x+12,pos.y+10);
        m_array->operator[](1).position = sf::Vector2f(pos.x+12,pos.y);
        m_array->operator[](2).position = sf::Vector2f(pos.x+5,pos.y+6);
        m_array->operator[](3).position = sf::Vector2f(pos.x+15,pos.y+6);
        m_array->operator[](0).color = sf::Color(255, 153, 0);
        m_array->operator[](1).color = sf::Color(255, 153, 0);
        m_array->operator[](2).color = sf::Color(255, 153, 0);
        m_array->operator[](3).color = sf::Color(255, 153, 0);
    }
    else if(i==5)
    {
        m_array = std::make_unique<sf::VertexArray>(sf::LinesStrip,6);
        m_array->operator[](1).position = sf::Vector2f(pos.x+5,pos.y);
        m_array->operator[](0).position = sf::Vector2f(pos.x+15,pos.y);
        m_array->operator[](3).position = sf::Vector2f(pos.x+15,pos.y+5);
        m_array->operator[](2).position = sf::Vector2f(pos.x+5,pos.y+5);
        m_array->operator[](5).position = sf::Vector2f(pos.x+5,pos.y+10);
        m_array->operator[](4).position = sf::Vector2f(pos.x+15,pos.y+10);
        m_array->operator[](0).color = sf::Color(255, 153, 0);
        m_array->operator[](1).color = sf::Color(255, 153, 0);
        m_array->operator[](2).color = sf::Color(255, 153, 0);
        m_array->operator[](3).color = sf::Color(255, 153, 0);
        m_array->operator[](4).color = sf::Color(255, 153, 0);
        m_array->operator[](5).color = sf::Color(255, 153, 0);
    }
    else if(i==6)
    {
        m_array = std::make_unique<sf::VertexArray>(sf::LinesStrip,6);
        m_array->operator[](0).position = sf::Vector2f(pos.x+15,pos.y);
        m_array->operator[](1).position = sf::Vector2f(pos.x+5,pos.y);
        m_array->operator[](2).position = sf::Vector2f(pos.x+5,pos.y+10);
        m_array->operator[](3).position = sf::Vector2f(pos.x+15,pos.y+10);
        m_array->operator[](4).position = sf::Vector2f(pos.x+15,pos.y+5);
        m_array->operator[](5).position = sf::Vector2f(pos.x+5,pos.y+5);
        m_array->operator[](0).color = sf::Color(255, 153, 0);
        m_array->operator[](1).color = sf::Color(255, 153, 0);
        m_array->operator[](2).color = sf::Color(255, 153, 0);
        m_array->operator[](3).color = sf::Color(255, 153, 0);
        m_array->operator[](4).color = sf::Color(255, 153, 0);
        m_array->operator[](5).color = sf::Color(255, 153, 0);
    }
    else if (i==7)
    {
        m_array = std::make_unique<sf::VertexArray>(sf::LinesStrip,3);
        m_array->operator[](0).position = sf::Vector2f(pos.x+5,pos.y);
        m_array->operator[](1).position = sf::Vector2f(pos.x+15,pos.y);
        m_array->operator[](2).position = sf::Vector2f(pos.x+5,pos.y+10);
        m_array->operator[](0).color = sf::Color(255, 153, 0);
        m_array->operator[](1).color = sf::Color(255, 153, 0);
        m_array->operator[](2).color = sf::Color(255, 153, 0);
    }
    else if(i==8)
    {
        m_array = std::make_unique<sf::VertexArray>(sf::LinesStrip,7);
        m_array->operator[](0).position = sf::Vector2f(pos.x+15,pos.y+5);
        m_array->operator[](1).position = sf::Vector2f(pos.x+15,pos.y);
        m_array->operator[](2).position = sf::Vector2f(pos.x+5,pos.y);
        m_array->operator[](3).position = sf::Vector2f(pos.x+5,pos.y+10);
        m_array->operator[](4).position = sf::Vector2f(pos.x+15,pos.y+10);
        m_array->operator[](5).position = sf::Vector2f(pos.x+15,pos.y+5);
        m_array->operator[](6).position = sf::Vector2f(pos.x+5,pos.y+5);
        m_array->operator[](0).color = sf::Color(255, 153, 0);
        m_array->operator[](1).color = sf::Color(255, 153, 0);
        m_array->operator[](2).color = sf::Color(255, 153, 0);
        m_array->operator[](3).color = sf::Color(255, 153, 0);
        m_array->operator[](4).color = sf::Color(255, 153, 0);
        m_array->operator[](5).color = sf::Color(255, 153, 0);
        m_array->operator[](6).color = sf::Color(255, 153, 0);
    }

}
////////////////////////////////////////////////////////////////////////////////////
void CNumber::Draw()
{
    CGame::Instance().getWindow()->draw(*m_array);
}
///////////////////////////////////////////////////////////////////////////////////////

bool CBlock::IsPressed()
{
    if(sf::Mouse::isButtonPressed(sf::Mouse::Left)||sf::Mouse::isButtonPressed(sf::Mouse::Right))
    {
        sf::Vector2i localPosition = sf::Mouse::getPosition(*(CGame::Instance().getWindow()));
        sf::Vector2f m_position = shape.getPosition();
        if(localPosition.x>m_position.x&&localPosition.x<m_position.x+size.x)
        {
            if(localPosition.y>m_position.y&&localPosition.y<m_position.y+size.y)
            {
                return true;
            }
        }
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////////////
void CMineBlock::Update(const float &deltaTime)
{
    lastFrame += deltaTime;
    if(CBlock::GetStatus()==BlockStatus::show)
    {
        CBlock::SetColor(sf::Color::Red);
    }
    else
    {
        if(CBlock::IsPressed()&&lastFrame>0.2)
        {
            lastFrame=0;
            if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                if(CBlock::GetStatus()!=BlockStatus::set)
                {
                    // lose...
                    CBlock::SetColor(sf::Color::Red);
                    CBlock::SetStatus(BlockStatus::show);
                    CGame::Instance().setStatus(Status::lose);
                }
            }
            if(sf::Mouse::isButtonPressed(sf::Mouse::Right))
            {
                if(CBlock::GetStatus()!=BlockStatus::set)
                {
                    CBlock::SetColor(sf::Color::Magenta);
                    CBlock::SetStatus(BlockStatus::set);
                }
                else
                {
                    CBlock::SetColor(sf::Color::Blue);
                    CBlock::SetStatus(BlockStatus::hide);
                }
            }

        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////
void CSaveBlock::Update(const float &deltaTime)
{
    lastFrame+=deltaTime;
    if(CBlock::IsPressed()&&lastFrame>0.2)
    {
        lastFrame=0;
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            if(CBlock::GetStatus()!=BlockStatus::set)
            {
                CBlock::SetStatus(BlockStatus::show);
                CBlock::SetColor(sf::Color::Green);
            }
        }
        else if(sf::Mouse::isButtonPressed(sf::Mouse::Right)&& CBlock::GetStatus()!=BlockStatus::show)
        {
            if(CBlock::GetStatus()!=BlockStatus::set )
            {
                CBlock::SetColor(sf::Color::Magenta);
                CBlock::SetStatus(BlockStatus::set);
            }
            else
            {
                CBlock::SetColor(sf::Color::Blue);
                CBlock::SetStatus(BlockStatus::hide);
            }
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////
void CMap::Happy()
{
    for(auto i = 0u; i<TBlocks.size(); i++)
    {
        int a = std::rand()%3;
        if(a==1) TBlocks[i]->SetColor(sf::Color::Magenta);
        if(a==2) TBlocks[i]->SetColor(sf::Color::Blue);
    }
}
///////////////////////////////////////////////////////////////////////////////////////
void CMap::Generate(sf::Vector2f pos)
{
    int current;
    for(int count = 0; count<10; count++)
    {
        current = std::rand()%64;
        for(int i=0; i<count; i++)
        {
            if(random[i]==current)
            {
                i=-1;
                current = std::rand()%64;
            }
        }
        random[count]=current;
    }
    for(int i=0; i<10; i++)
        for(int j=1; j<10-i; j++)
            if(random[j-1]>random[j])
            {
                current = random[j-1];
                random[j-1] = random[j];
                random[j] = current;
            }
    current = 0;
    std::shared_ptr<CBlock> TCurrentBlock;
    std::shared_ptr<CMineBlock> TCurrentMineBlock;
    std::shared_ptr<CSaveBlock> TCurrentSaveBlock;
    for(int i=0; i<64; i++)
    {
        if(i==random[current])
        {
            TCurrentMineBlock = std::make_shared<CMineBlock>();
            TCurrentBlock = TCurrentMineBlock;
            m_mines->Add(TCurrentMineBlock);
            current++;
        }
        else
        {
            TCurrentSaveBlock = std::make_shared<CSaveBlock>();
            TCurrentBlock = TCurrentSaveBlock;
            m_saveBlocks->Add(TCurrentSaveBlock);
        }
        TBlocks.push_back(TCurrentBlock);
    }
    int x = 50,y=50;
    for(int i=0; i<64; i++)
    {
        TBlocks[i]->SetPosition(sf::Vector2f(x,y));
        x+=50;
        if(x==450)
        {
            x=50;
            y+=50;
        }
    }
    for(int i=0; i<10; i++)
    {
        if(random[i]-1>=0&&random[i]%8!=0) TBlocks[random[i]-1]->ImproveNumber();
        if(random[i]+1<64&&(random[i]+1)%8!=0) TBlocks[random[i]+1]->ImproveNumber();
        if(random[i]-9>=0&&random[i]%8!=0) TBlocks[random[i]-9]->ImproveNumber();
        if(random[i]-8>=0) TBlocks[random[i]-8]->ImproveNumber();
        if(random[i]-7>=0&&(random[i]+1)%8!=0&&random[i]!=0) TBlocks[random[i]-7]->ImproveNumber();
        if(random[i]+7<64&&random[i]%8!=0) TBlocks[random[i]+7]->ImproveNumber();
        if(random[i]+8<64) TBlocks[random[i]+8]->ImproveNumber();
        if(random[i]+9<64&&((random[i]+1)%8!=0||random[i]==0)) TBlocks[random[i]+9]->ImproveNumber();
    }
    GenerateNumber(*m_saveBlocks);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    srand(time(NULL));
    auto &game = CGame::Instance();
    game.Run();
    return 0;
}

