#include "AlgorithmBase.h"

#include <algorithm>
#include <cstring>
#include <queue>

AlgorithmBase::AlgorithmBase()
{
	_house = new char*[MAXHOUSELENGTH];
	for (size_t i = 0; i < MAXHOUSELENGTH; i++)
	{
		_house[i] = new char[MAXHOUSELENGTH + 1];
		_house[i][MAXHOUSELENGTH] = '\0';
		memset(_house[i], AlgorithmBase::UNKNOWN, MAXHOUSELENGTH); // fill in all places with unknowns
	}

	_robot.location = Point(_houseLength / 2, _houseHeight / 2);
	_docking = Point(_houseLength / 2, _houseHeight / 2);

}

void AlgorithmBase::freeHouse(int height)
{
	if (_house != nullptr)
	{
		for (int i = 0; i < height; i++)
		{
			delete[] _house[i];
		}

		delete[] _house;
		_house = nullptr;
	}
}

AlgorithmBase::~AlgorithmBase()
{
	freeHouse(_houseHeight);
}

void AlgorithmBase::aboutToFinish(int stepsTillFinishing_)
{
	_aboutToFinishCalled = true;
	_movesUntilFinish = stepsTillFinishing_;
}
	
void AlgorithmBase::dijakstra(Point dest_, vector<Direction>& result_)
{
	queue<Point> queue;
	set<Point> untouched;
	map<Point, Direction> map;

	for (int i = 0; i < _houseHeight; ++i)
	{
		for (int j = 0; j < _houseLength; ++j)
		{
			char block = _house[i][j];
			if (block != UNKNOWN && block != WALL)
			{
				untouched.insert(Point(j, i));
			}
		}
	}

	queue.push(_robot.location);
	untouched.erase(_robot.location);

	bool foundPath = false;
	while (!foundPath && queue.size() > 0)
	{
		Point point = queue.front();
		queue.pop();

		for (int i = 0; i < 4; i++)
		{
			Point block = point;
			block.move((Direction)i);

			if (untouched.find(block) != untouched.end())
			{
				untouched.erase(block);
				queue.push(block);
				
				map[block] = ((Direction) i);
			}

			if (block == dest_)
			{
				foundPath = true;
			}
		}
	}

	if (!foundPath)
	{
		// nothing to do
		return;
	}

	// storing path to destination
	result_.clear();
	Point p = dest_;

	while (!(p == _robot.location))
	{
		Direction dir = map[p];
		result_.push_back(dir);
		p.move(oppositeDirection(dir));
	}
}

int AlgorithmBase::calcScoreForPath(int untilPoint, Point point)
{
	int addedValue = 1000000;
	char c = _house[point.getY()][point.getX()];
	if (c == EMPTY || c == CLEAN)
	{
		addedValue = 10;
	}
	else if (c == NOTWALL)
	{
		addedValue = 9;
	}
	else if (c >= DUST1 && c<= DUST9)
	{
		addedValue = DUST9 - c;
	}

	return untilPoint + addedValue + 100000;
}

void AlgorithmBase::dijakstraHome(Point dest_, vector<Direction>& result_)
{

	// I'd like to take a minute to explain all data structures in use:

	// Normal dijkstra queue, all items can be here max 1 time
	queue<Point> queue;
	// all items in queue, used for finding in O(1)
	set<Point> inQueue;
	// all items that were taken out of the queue, or points we'll not travel to
	set<Point> dontTouch;
	// scoring map, includeing direction which we came from
	map<Point, tuple<int, Direction>> map;

	for (int i = 0; i < _houseHeight; ++i)
	{
		for (int j = 0; j < _houseLength; ++j)
		{
			char block = _house[i][j];
			if (block == UNKNOWN || block == WALL)
			{
				dontTouch.insert(Point(j, i));
			}
			// dont touch the frame too
			if (i == 0 || j == 0 || i == _houseHeight - 1 || j == _houseLength - 1)
			{
				dontTouch.insert(Point(j, i));
			}
		}
	}

	queue.push(_robot.location);
	inQueue.insert(_robot.location);
	dontTouch.insert(_robot.location);

	bool foundPath = false;
	while (!foundPath && queue.size() > 0)
	{
		Point point = queue.front();
		queue.pop();
		inQueue.erase(point);
		dontTouch.insert(point);

		if (point == dest_)
		{
			foundPath = true;
			break;
		}
		
		for (int i = 0; i < 4; i++)
		{
			Point block = point;
			block.move((Direction)i);

			if (dontTouch.find(block) != dontTouch.end())
			{
				// block is a wall, '?', or already computed it's weight
				continue;
			}

			if (inQueue.find(block) == inQueue.end())
			{
				// block is not in queue
				map[block] = tuple<int, Direction>(calcScoreForPath(std::get<0>(map[point]), point), (Direction)i);
				queue.push(block);
				inQueue.insert(block);
			}
			else
			{
				int score = calcScoreForPath(std::get<0>(map[point]), point);
				int prevScore = std::get<0>(map[block]);
				if (score < prevScore)
				{
					map[block] = tuple<int, Direction>(score, (Direction)i);
				}
			}
		}
	}

	if (!foundPath)
	{
		// nothing to do
		return;
	}

	// storing path to destination
	result_.clear();
	Point p = dest_;

	while (!(p == _robot.location))
	{
		Direction dir = std::get<1>(map[p]);
		result_.push_back(dir);
		p.move(oppositeDirection(dir));
	}
}


bool AlgorithmBase::isDocking() const
{
	return _robot.location == _docking;
}

Direction AlgorithmBase::oppositeDirection(Direction direction_)
{
	switch (direction_)
	{
		case Direction::East: return Direction::West;
		case Direction::West: return Direction::East;
		case Direction::North: return Direction::South;
		case Direction::South: return Direction::North;
		case Direction::Stay: return Direction::Stay;
	}
	return Direction::Stay;
}

void AlgorithmBase::updateBeforeMove(Direction choosenMove_)
{
	_lastMove = choosenMove_;
}

void AlgorithmBase::updatePointsSet(set<Point>& points, unsigned xOffset, unsigned yOffset)
{
	set<Point> newLocations;
	for (auto it = points.begin(); it != points.end(); ++it)
	{
		newLocations.insert(Point(it->getX() + xOffset, it->getY() + yOffset));
	}
	points = newLocations;
}

void AlgorithmBase::updatePoints(unsigned int xOffset, unsigned int yOffset)
{
	_robot.location = Point(_robot.location.getX() + xOffset, _robot.location.getY() + yOffset);

	updatePointsSet(_NLocations, xOffset, yOffset);
	updatePointsSet(_dirtyLocations, xOffset, yOffset);

	_docking = Point(_docking.getX() + xOffset, _docking.getY() + yOffset);
}

void AlgorithmBase::expandMatrix()
{
	if (_robot.location.getY() == _houseHeight -2)
	{
		int oldHeight = _houseHeight;

		_houseHeight *= 2;
		char** newHouse = new char*[_houseHeight];

		for (int i = 0; i < oldHeight; i++)
		{
			newHouse[i] = new char[_houseLength + 1];
			strncpy(newHouse[i], _house[i], _houseLength);
			newHouse[i][_houseLength] = '\0';
		}

		for (int i = oldHeight; i < _houseHeight; i++)
		{
			newHouse[i] = new char[_houseLength + 1];
			newHouse[i][_houseLength] = '\0';
			memset(newHouse[i], AlgorithmBase::UNKNOWN, _houseLength);
		}
		freeHouse(oldHeight);
		_house = newHouse;
	}


	if (_robot.location.getX() == _houseLength - 2)
	{
		unsigned oldLength = _houseLength;

		_houseLength *= 2;
		char** newHouse = new char*[_houseLength];

		for (int i = 0; i < _houseHeight; i++)
		{
			newHouse[i] = new char[_houseLength + 1];
			strncpy(newHouse[i], _house[i], oldLength);
			memset(newHouse[i] + oldLength, AlgorithmBase::UNKNOWN, oldLength);

			newHouse[i][_houseLength] = '\0';
		}

		freeHouse(_houseHeight);
		_house = newHouse;
	}

	if (_robot.location.getY() == 1)
	{
		int oldHeight = _houseHeight;

		_houseHeight *= 2;
		char** newHouse = new char*[_houseHeight];

		for (int i = 0; i < oldHeight; i++)
		{
			newHouse[i] = new char[_houseLength + 1];
			newHouse[i][_houseLength] = '\0';
			memset(newHouse[i], AlgorithmBase::UNKNOWN, _houseLength);
		}

		for (int i = oldHeight; i < _houseHeight; i++)
		{
			newHouse[i] = new char[_houseLength + 1];
			strncpy(newHouse[i], _house[i-oldHeight], _houseLength);
			newHouse[i][_houseLength] = '\0';
		}

		freeHouse(oldHeight);
		_house = newHouse;

		updatePoints(0, oldHeight);
	}

	if (_robot.location.getX() == 1)
	{
		unsigned oldLength = _houseLength;

		_houseLength *= 2;
		char** newHouse = new char*[_houseLength];

		for (int i = 0; i < _houseHeight; i++)
		{
			newHouse[i] = new char[_houseLength + 1];
			memset(newHouse[i] , AlgorithmBase::UNKNOWN, oldLength);
			strncpy(newHouse[i] + oldLength, _house[i], oldLength);

			newHouse[i][_houseLength] = '\0';
		}

		freeHouse(_houseHeight);
		_house = newHouse;

		updatePoints(oldLength, 0);
	}
}

void AlgorithmBase::updateAfterMove(Direction direction_)
{
	// update robot info
	_robot.location.move(direction_);

	if (_house[_robot.location.getY()][_robot.location.getX()] == DUST1)
	{
		_dirtyLocations.erase(_robot.location);
	}

	_robot.totalSteps++;

	expandMatrix();
	
	dijakstraHome(_docking, _dijakstraHome);
	updateRemainingMoves();

	updateBattery();
}

Direction AlgorithmBase::recoverFromUndisciplinedRobot(Direction actualPrevStep_, SensorInformation info, vector<Direction>& possibleMoves)
{
	if (actualPrevStep_ == Direction::Stay)
	{
		return _lastMove;
	}

	if (_mode == DIJAKSTRA || _mode == SCAN)
	{
		_mode = SCAN;
		return getMoveScanMode(info, possibleMoves);
	}
	else if (_mode == RETURNHOME || _mode == LOWBATTERY)
	{
		return getMoveReturnHomeMode(possibleMoves);
	}

	return oppositeDirection(actualPrevStep_);
}

string AlgorithmBase::DirectionToString(Direction direction) const
{
	switch (direction)
	{
	case Direction::Stay: return "stay";
	case Direction::East: return "east";
	case Direction::West: return "west";
	case Direction::North: return "north";
	case Direction::South: return "south";
	}
	return "NULL";
}

void AlgorithmBase::updateHouseKnowladge(SensorInformation info)
{
	for (unsigned int i = 0; i < sizeof(info.isWall) / sizeof(bool); ++i)
	{
		Direction direction = (Direction) i;
		Point block = _robot.location;

		block.move(direction);

		if (_house[block.getY()][block.getX()] == UNKNOWN)
		{
			if (info.isWall[i])
			{
				_house[block.getY()][block.getX()] = WALL;
			}
			else
			{
				_house[block.getY()][block.getX()] = NOTWALL;
				_NLocations.insert(block);
			}
		}
	}

	int dL = info.dirtLevel;
	if (dL > 1)
	{
		_dirtyLocations.insert(_robot.location);
	}

	_NLocations.erase(_robot.location);
	_house[_robot.location.getY()][_robot.location.getX()] = (dL == 0) ? EMPTY : CLEAN + dL;
}

Point AlgorithmBase::findClosestPoint(const set<Point>& points)
{
	Point result;
	int minDistance = 10000000;
	for (auto it = points.begin(); it != points.end(); ++it)
	{
		vector<Direction> dirsToPoint;
		dijakstra(*it, dirsToPoint);
		int distance = dirsToPoint.size();
		if (distance < minDistance)
		{
			result = *it;
			minDistance = distance;
		}
	}
	return result;
}

int AlgorithmBase::GetMovesToPoint(Point point)
{
	vector<Direction> directions;
	dijakstra(point, directions);

	return directions.size();
}

Direction AlgorithmBase::getMoveScanMode(SensorInformation info, vector<Direction>& possiblemoves)
{
	if (info.dirtLevel > 0)
	{
		return Direction::Stay;
	}
	else
	{
		_dirtyLocations.erase(_robot.location);
	}

	bool findNearBlock = false;
	int maxDirtFound = -1;
	Direction result = Direction::Stay;

	for (auto it = possiblemoves.begin(); it != possiblemoves.end(); it++)
	{
		// looking for new block
		Point block = _robot.location;
		block.move(*it);

		if (_NLocations.find(block) != _NLocations.end()){
			if(maxDirtFound < 0){
				maxDirtFound = 0;
				result = *it;
			}
			findNearBlock = true;
		}
		else if (_dirtyLocations.find(block) != _dirtyLocations.end())
		{
			int dirt = _house[block.getY()][block.getX()] - DUST1 +1;
			if (dirt > maxDirtFound)
			{
				maxDirtFound = dirt;
				result = *it;
			}
			findNearBlock = true;
		}		
	}

	if (findNearBlock)
	{
		return result;
	}

	//	else go To Some N or go home

	if (_NLocations.size() == 0 && _dirtyLocations.size() == 0)
	{
		_mode = RETURNHOME;
		return getMoveReturnHomeMode(possiblemoves);
	}
	else
	{
		_mode = DIJAKSTRA;

		if (_NLocations.size() == 0)
		{
			dijakstra(findClosestPoint(_dirtyLocations), _dijakstraToDest);
			return getMoveDijakstraMode(possiblemoves);
		}

		if (_dirtyLocations.size() == 0)
		{
			dijakstra(findClosestPoint(_NLocations), _dijakstraToDest);
			return getMoveDijakstraMode(possiblemoves);
		}

		Point NDest = findClosestPoint(_NLocations);
		Point dirtyDest = findClosestPoint(_dirtyLocations);
		int movesToNDest = GetMovesToPoint(NDest);
		int movesTodirtyDest = GetMovesToPoint(dirtyDest);
		int dirtyLevel = _house[dirtyDest.getY()][dirtyDest.getX()] - DUST1 + 1;

		Point dest = dirtyDest;
		if (movesTodirtyDest > movesToNDest + 2 * dirtyLevel)
		{
			dest = NDest;
		}

		dijakstra(dest, _dijakstraToDest);
		return getMoveDijakstraMode(possiblemoves);
	}
}

size_t AlgorithmBase::movesUntilNoBattery()
{
	return (size_t) _robot.battery / _config["BatteryConsumptionRate"];
}

Direction AlgorithmBase::getMoveDijakstraMode(vector<Direction>& vector)
{
	if (_dijakstraToDest.size() > movesUntilNoBattery() )
	{
		_mode = RETURNHOME;
		return getMoveReturnHomeMode(vector);
	}

	if (_dijakstraToDest.size() == 1)
	{
		_mode = SCAN;
	}

	Direction dir = _dijakstraToDest.back();
	_dijakstraToDest.pop_back();

	return dir;
}

Direction AlgorithmBase::getMoveReturnHomeMode(vector<Direction>& vector)
{
	if (_robot.location == _docking)
	{
		return Direction::Stay;
	}

	Direction dir = _dijakstraHome.back();
	_dijakstraHome.pop_back();

	return dir;
}

Direction AlgorithmBase::getMove(Direction prevStep_, vector<Direction>& order_)
{
	SensorInformation info = _sensor->sense(); // info.isWall = { East, West, South, North }
	updateHouseKnowladge(info);

	vector<Direction> possibleMoves;
	for (auto dir : order_)
	{
		if (!info.isWall[(int)dir])
		{
			possibleMoves.push_back(dir);
		}
	}

	if (prevStep_ != _lastMove)
	{
		_undisciplinedCount++;
		return recoverFromUndisciplinedRobot(prevStep_, info, possibleMoves);
	}

	// TODO: battery Check /////////////
	if (_mode == LOWBATTERY)
	{
		return getMoveReturnHomeMode(possibleMoves);
	}

	if (_mode == SCAN)
	{
		return getMoveScanMode(info, possibleMoves);
	}

	if (_mode == DIJAKSTRA)
	{
		return getMoveDijakstraMode(possibleMoves);
	}

	if (_mode == RETURNHOME)
	{
		return getMoveReturnHomeMode(possibleMoves);
	}

	return possibleMoves[0];
}

void AlgorithmBase::printHouse(Point robotLocation) const
{
	cout << "####################################################" << endl;
	cout << "Guessed House : " << endl;
	char prevR = _house[robotLocation.getY()][robotLocation.getX()];
	char prevD = _house[_docking.getY()][_docking.getX()];
	_house[robotLocation.getY()][robotLocation.getX()] = 'R';
	_house[_docking.getY()][_docking.getX()] = 'D';
	for (int i = 0; i < _houseHeight; i++)
	{
		cout << _house[i] << endl;
	}
	_house[robotLocation.getY()][robotLocation.getX()] = prevR;
	_house[_docking.getY()][_docking.getX()] = prevD;
	cout << "####################################################" << endl;
}

void AlgorithmBase::printNLocation()
{
	cout << "Printing Set:" << endl;
	for (auto iter = _NLocations.begin(); iter != _NLocations.end(); ++iter){
		cout<<(*iter) <<endl;
	}
}

void AlgorithmBase::updateRemainingMoves()
{
	_movesUntilFinish--;
}

size_t AlgorithmBase::NumberOfMovesToDocking()
{
	int actualSteps = _dijakstraHome.size();
	double undisciplinedRate = getUndisciplinedRate();
	return (size_t)(actualSteps * (1 + undisciplinedRate));
}

double AlgorithmBase::getUndisciplinedRate()
{
	return (_undisciplinedCount / (double)_robot.totalSteps) + 0.02;
}

void AlgorithmBase::updateBattery()
{
	int consumptionRate = _config["BatteryConsumptionRate"];
	int rechargeRate = _config["BatteryRechargeRate"];
	int capacity = _config["BatteryCapacity"];

	if (isDocking())
	{
		_robot.battery = std::min(_robot.battery + rechargeRate, capacity);
	}
	else
	{
		_robot.battery -= consumptionRate;
	}

	int movesToDocking = NumberOfMovesToDocking();
	int returnBatteryConsumption = movesToDocking * consumptionRate;

	// Next were figuring out if we have enough battery to return home
	// The 2 marked here is a safety margin //////////////////////////   -|-  /////////////////////
	if ((_robot.battery >= returnBatteryConsumption) && (_robot.battery - 2 * consumptionRate <= returnBatteryConsumption))
	{
		_mode = LOWBATTERY;
		return;
	}

	if (_robot.battery == capacity)
	{
		_mode = SCAN;
	}

	// The 3 marked here is a safety margin /////-|-/////////////////////
	if (_aboutToFinishCalled && (movesToDocking + 3 > _movesUntilFinish) && ((int)_dijakstraHome.size() < _movesUntilFinish))
	{
		_mode = RETURNHOME;
	}
}
