#pragma once
#include "../stdafx.h"
#include "BehaviorTree.h"

#pragma region Functions
inline BehaviorState UseRestoreItem(Blackboard* pBlackBoard, eItemType type, string metadata, float healtOrEnergyValue)
{
#pragma region Get_Info_From_Blackboard
	//Get the interface
	IExamInterface* pInterface = nullptr;
	pBlackBoard->GetData("Interface", pInterface);

	//Get the inventory
	std::vector<ItemInfo> inventory = {};
	pBlackBoard->GetData("Inventory", inventory);

	//Get the max health/energy value
	float maxValue = 0;
	pBlackBoard->GetData("MaxHealthAndEnergyValue", maxValue);

#pragma endregion

	ItemInfo smallestRestore = {};

	//Find a restore of the given type
	auto it = std::find_if(inventory.begin(), inventory.end(), [&](ItemInfo inventoryItem)
	{
		return inventoryItem.Type == type;
	});

	//If there is restore found
	if (it != inventory.end())
	{
		//Set the found restore as the smallest one
		int indexOfSmallestRestore = std::distance(inventory.begin(), it);
		pInterface->Inventory_GetItem(indexOfSmallestRestore, smallestRestore);

		//Get how much it restores
		int smallestRestoreAmmount = pInterface->Item_GetMetadata(smallestRestore, metadata);

		//Look if there is a smaller restore
		for (int i = 0; i < inventory.size(); i++)
		{
			ItemInfo emptyItem = {};

			//Check if the spot is not empty (personal inventory)
			if (inventory[i].Location != emptyItem.Location)
			{
				//Get the item in the slot of the interface
				ItemInfo inventoryItem = {};
				pInterface->Inventory_GetItem(i, inventoryItem);

				//Check if the item is off the type we need
				if (inventoryItem.Type == type)
				{
					//Get the restore ammount of the item in the interface inventory
					int inventoryRestoreAmmount = pInterface->Item_GetMetadata(inventoryItem, metadata);

					//check if the restore ammount is less
					if (inventoryRestoreAmmount < smallestRestoreAmmount)
					{
						//Set the new smallest restore
						smallestRestore = inventoryItem;
						smallestRestoreAmmount = inventoryRestoreAmmount;
						indexOfSmallestRestore = i;
					}
				}
			}
		}

		//The smallest restore is found, 
		//		check if it doesn't overRestore if we use it
		if (healtOrEnergyValue + smallestRestoreAmmount < maxValue)
		{
			//Use the item
			pInterface->Inventory_UseItem(indexOfSmallestRestore);

			//Delete the item from the inventory
			ItemInfo emptyItem = {};
			inventory[indexOfSmallestRestore] = emptyItem;
			pBlackBoard->ChangeData("Inventory", inventory);

			pInterface->Inventory_RemoveItem(indexOfSmallestRestore);

			return Success;
		}
	}
	return Failure;
}

inline Elite::Vector2 GetClosestCorner(Elite::Vector2 currentPoint, std::vector<Elite::Vector2> &corners)
{
	Elite::Vector2 closestCorner = corners[0];

	for each (Elite::Vector2 corner in corners)
	{
		if (corner != currentPoint)
		{
			if (Elite::Distance(currentPoint, corner) < Elite::Distance(currentPoint, closestCorner))
			{
				closestCorner = corner;
			}
		}
	}
	
	corners.erase(std::remove_if(corners.begin(), corners.end(), [&](Elite::Vector2 corner) 
	{
		return corner == closestCorner;
	}), corners.end());

	return closestCorner;
}

inline Elite::Vector2 GetFurthestCorner(Elite::Vector2 currentPoint, std::vector<Elite::Vector2> &corners)
{
	Elite::Vector2 furthestCorner = corners[0];

	for each (Elite::Vector2 corner in corners)
	{
		if (Elite::Distance(currentPoint, corner) > Elite::Distance(currentPoint, furthestCorner))
		{
			furthestCorner = corner;
		}
	}

	corners.erase(std::remove_if(corners.begin(), corners.end(), [&](Elite::Vector2 corner)
	{
		return corner == furthestCorner;
	}),corners.end());

	return furthestCorner;
}

inline int AmmountOfTypeInInventory(Blackboard* pBlackBoard, eItemType type)
{
#pragma region Get_Info_From_Blackboard
	//Get the inventory
	std::vector<ItemInfo> inventory = {};
	pBlackBoard->GetData("Inventory", inventory);

#pragma endregion
	//Times found in inventory
	int timesFound = 0;

	//Search inventory for Food
	for each (ItemInfo inventoryItem in inventory)
	{
		if (inventoryItem.Type == type)
		{
			timesFound++;
		}
	}
	return timesFound;
}

inline bool ReemoveItem(Blackboard* pBlackBoard, eItemType type)
{
#pragma region Get_Info_From_Blackboard
	//Get the intereface
	IExamInterface* pInterface = {};
	pBlackBoard->GetData("Interface", pInterface);

	//Get inventory
	std::vector<ItemInfo> inventory = {};
	pBlackBoard->GetData("Inventory", inventory);

	//Get memory
	std::vector<ItemInfo> itemsInMemory = {};
	pBlackBoard->GetData("ItemsInMemory", itemsInMemory);

	//Get agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

#pragma endregion

	//Get first index of type
	auto it = std::find_if(inventory.begin(), inventory.end(), [&](ItemInfo itemInInventory)
	{
		return itemInInventory.Type == type;
	});

	//Make sure an item is found in the personal inventory of th givn type
	if (it != inventory.end())
	{
		//Determine the index
		int index = std::distance(inventory.begin(), it);

		//Empty Item
		ItemInfo emptyItem = {};

		//Clear inventory of item
		inventory[index] = emptyItem;
		pBlackBoard->ChangeData("Inventory", inventory);

		pInterface->Inventory_RemoveItem(index);

		return true;
	}

	//There was no item of the type, --> no drop, return false
	return false;
}

inline bool IsPointInHouse(Elite::Vector2 &point, HouseInfo house)
{
	house.Size /= 2.0f;

	Elite::Vector2 leftTop = { house.Center.x - house.Size.x, house.Center.y + house.Size.y };
	Elite::Vector2 rightTop = { house.Center.x + house.Size.x, house.Center.y + house.Size.y };
	Elite::Vector2 leftBot = { house.Center.x - house.Size.x, house.Center.y - house.Size.y };
	Elite::Vector2 rightBot = { house.Center.x + house.Size.x, house.Center.y - house.Size.y };

	if (point.x > leftTop.x && point.y < leftTop.y &&
		point.x < rightTop.x && point.y < rightTop.y &&
		point.x > leftBot.x && point.y > leftBot.y &&
		point.x < rightBot.x && point.y > rightBot.y)
	{
		return true;
	}
	
	std::cout << "EnemyLocation: " << point.x << ", " << point.y << std::endl;
	std::cout << "LeftTop: " << leftTop.x << ", " << leftTop.y << std::endl;
	std::cout << "RightTop: " << rightTop.x << ", " << rightTop.y << std::endl;
	std::cout << "LeftBot: " << leftBot.x << ", " << leftBot.y << std::endl;
	std::cout << "RightBot: " << rightBot.x << ", " << rightBot.y << std::endl;

	return false;
}
#pragma endregion

#pragma region SteeringBehaviors
inline BehaviorState Seek(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the Interface
	IExamInterface* pInterface = nullptr;
	pBlackBoard->GetData("Interface", pInterface);

	//Get the target location
	Elite::Vector2 target = {};
	pBlackBoard->GetData("Target", target);

	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);
#pragma endregion

	//Use the navmesh to calculate the next navmesh point
	auto nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(target);

	//Create a steering output
	SteeringPlugin_Output steering = {};

	//Simple Seek Behaviour (towards Target)
	steering.LinearVelocity = nextTargetPos - agent.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agent.MaxLinearSpeed; //Rescale to Max Speed

	//Get wether the agent should auto orientate from the BlackBoard
	bool shouldAutoOrientate = false;
	pBlackBoard->GetData("AutoOrientate", shouldAutoOrientate);

	//Set Orientation
	steering.AutoOrientate = shouldAutoOrientate; //Setting AutoOrientate to TRue overrides the AngularVelocity

	if (!shouldAutoOrientate)
	{
		float rotationValue = 0.0f;
		pBlackBoard->GetData("RotationValue", rotationValue);
		steering.AngularVelocity = rotationValue;
	}

	//Get wether the agent is resting or not
	bool isResting = false;
	pBlackBoard->GetData("IsResting", isResting);

	//Check if the agent is resting
	if (isResting)
	{
		//Disable runspeed --> Walk
		steering.RunMode = false; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)
	}
	else
	{
		//Get if the agent canRun
		bool canRun = false;
		pBlackBoard->GetData("CanRun", canRun);

		//Set the runmode to result of canRun
		steering.RunMode = canRun; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)
	}

	//Change the steering data
	pBlackBoard->ChangeData("Steering", steering);

	return Success;
}

inline BehaviorState Evade(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the target location
	Elite::Vector2 target = {};
	pBlackBoard->GetData("Target", target);

	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

	//Get the closestEnemy
	EnemyInfo enemy = {};
	pBlackBoard->GetData("ClosestEnemy", enemy);

	float minEvadeAngleInDegree = 0;
	pBlackBoard->GetData("MinEvadeAngle", minEvadeAngleInDegree);
#pragma endregion

	//Calculate the angle between the forward and the directionToEnemy
	auto forward = agent.LinearVelocity.GetNormalized();
	auto directionToEnemy = (enemy.Location - agent.Position).GetNormalized();

	//Make sure the evade is always atleast 30degree to the other side
	auto minEvadeAngle = _Pi / 180 * minEvadeAngleInDegree;
	auto angle = atan2(forward.x*directionToEnemy.y - forward.y*directionToEnemy.x, forward.x*directionToEnemy.x + forward.y*directionToEnemy.y);

	//Add a minumum dodge angle
	angle += angle > 0 ? minEvadeAngle : -minEvadeAngle;

	//Set the angle as max to the FOV angle
	if (abs(angle) > agent.FOV_Angle)
	{
		if (angle < 0)
		{
			angle = agent.FOV_Angle / 2;
			angle *= -1;
		}
		else
		{
			angle = agent.FOV_Angle / 2;
		}
	}

	//Invert the angle so we dodge to the other side of the incoming vector
	angle *= -1;

	//Determine a minimum radius for the target distance
	auto minRadius = agent.FOV_Range / 4;

	//Set the new target as the forward rotated with the dodge angle
	Elite::Vector2 newTarget = { agent.Position.x + (minRadius * forward.x * cos(angle) - minRadius * forward.y * sin(angle)), agent.Position.y + (minRadius * forward.x * sin(angle) + minRadius * forward.y * cos(angle)) };

	//Edit the target data in the BlackBoard
	pBlackBoard->ChangeData("Target", newTarget);

	//Seek to the new target
	Seek(pBlackBoard);

	return Success;
}
#pragma endregion

#pragma region Conditionals
#pragma region Shooting
inline bool AreEnemiesInFOV(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the data of all entities in FOV
	std::vector<EntityInfo> entitiesInFOV = {};
	pBlackBoard->GetData("EntitiesInFOV", entitiesInFOV);

	//Get the interface
	IExamInterface* pExamInterface = nullptr;
	pBlackBoard->GetData("Interface", pExamInterface);

	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);
#pragma endregion

	//Get all enemy entities in FOV
	std::vector<EnemyInfo> enemiesInFOV = {};

	//Loop and convert
	for each (EntityInfo entity in entitiesInFOV)
	{
		if (entity.Type == eEntityType::ENEMY)
		{
			//Get the enemyInfo
			EnemyInfo enemyInfo = {};
			pExamInterface->Enemy_GetInfo(entity, enemyInfo);
	
			//Add the enemy to the FOV enemy vector
			enemiesInFOV.push_back(enemyInfo);
		}
	}

	//If there are no enmies return false
	if (enemiesInFOV.empty())
	{
		EnemyInfo emptyEnemy = {};
		pBlackBoard->ChangeData("ClosestEnemy", emptyEnemy);
		//Enable auto orientate
		pBlackBoard->ChangeData("AutoOrientate", true);

		return false;
	}
#pragma region GetClosest
	//Set the closest Enemy in the blackboard
	else
	{
		//GEt the first enemy
		EnemyInfo closestEnemy = enemiesInFOV[0];
		float closestEnemyDistance = Distance(closestEnemy.Location, agent.Position);

		//If there are multiple enmies search for the closest one
		if (enemiesInFOV.size() > 1)
		{
			//Iterate over all enmies and compare them with the first one
			for each (EnemyInfo enemy in enemiesInFOV)
			{
				auto disToEnemy = Distance(enemy.Location, agent.Position);

				if (disToEnemy < closestEnemyDistance)
				{
					closestEnemyDistance = disToEnemy;
					closestEnemy = enemy;
				}
			}
		}

		//Change the closest enemy data to the new closest enemy
		pBlackBoard->ChangeData("ClosestEnemy", closestEnemy);
	}
#pragma endregion

	return true;
}

inline bool CanShootAtEnemy(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//GEt the inventory
	std::vector<ItemInfo> inventory = {};
	pBlackBoard->GetData("Inventory", inventory);

	//Get the closestemeny
	EnemyInfo closestEnemy = {};
	pBlackBoard->GetData("ClosestEnemy", closestEnemy);

	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

	//GEt the interface
	IExamInterface* pInterface = nullptr;
	pBlackBoard->GetData("Interface", pInterface);

#pragma endregion

	//Check if the closestTarget is valid
	if (closestEnemy.EnemyHash != 0)
	{
		//Disable auto orientate
		pBlackBoard->ChangeData("AutoOrientate", false);

		//Search for a pistol in the inventory
		auto it = std::find_if(inventory.begin(), inventory.end(), [](ItemInfo itemInInventory)
		{
			return itemInInventory.Type == eItemType::PISTOL && itemInInventory.ItemHash != 0;
		});

		//If there is a gun found
		if (it != inventory.end())
		{
			//Set the found gun as the best gun
			int bestGunIndex = std::distance(inventory.begin(), it);
			ItemInfo bestGun = {};
			pInterface->Inventory_GetItem(bestGunIndex, bestGun);

			pBlackBoard->ChangeData("BestPistolIndex", bestGunIndex);
			printf("Gun Found\n");
			return true;
		}
		//Enable auto orientate since we have no gun
		pBlackBoard->ChangeData("AutoOrientate", true);
	}
	printf("AAAAAAH I have no gun!");
	return false;
}

inline bool IsEnemyInHouse(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the current house
	HouseInfo currentHouse = {};
	pBlackBoard->GetData("CurrentHouse", currentHouse);

	//Get the closestEnemy
	EnemyInfo closestEnemy = {};
	pBlackBoard->GetData("ClosestEnemy", closestEnemy);

#pragma endregion

	if (IsPointInHouse(closestEnemy.Location, currentHouse))
	{
		printf("Enemy is in house\n");
		return true;
	}

	printf("EnemyNotInHOuse\n");
	return false;
}
#pragma endregion

#pragma region SpecificItemFind
inline bool LowOnEnergy(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

	//GEt Inventory
	std::vector<ItemInfo> inventory = {};
	pBlackBoard->GetData("Inventory", inventory);

	//Get wantedAmmountOfFood
	int wantedAmmountOfFood = 0;
	pBlackBoard->GetData("WantedAmmountOfFood", wantedAmmountOfFood);

#pragma endregion

	//Check if there are less than wantedammount in the inventory
	if (AmmountOfTypeInInventory(pBlackBoard, eItemType::FOOD) < wantedAmmountOfFood)
	{
		//Set the blackboard data
		pBlackBoard->ChangeData("NeededItemType", eItemType::FOOD);
		return true;
	}
	return false;
}

inline bool LowOnHealth(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

	//GEt Inventory
	std::vector<ItemInfo> inventory = {};
	pBlackBoard->GetData("Inventory", inventory);

	//Get wantedAmmountOfMedkits
	int wantedAmmountOfMedkits = 0;
	pBlackBoard->GetData("WantedAmmountOfMedkits", wantedAmmountOfMedkits);

#pragma endregion

	//Check if there are less than 1 Medkits in the inventory
	if (AmmountOfTypeInInventory(pBlackBoard, eItemType::MEDKIT) < wantedAmmountOfMedkits)
	{
		//Set the blackboard data
		pBlackBoard->ChangeData("NeededItemType", eItemType::MEDKIT);
		return true;
	}
	return false;
}

inline bool LowOnPistols(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get inventory
	std::vector<ItemInfo> inventory;
	pBlackBoard->GetData("Inventory", inventory);

	//Get wantedAmmountofPistols
	int wantedAmmountOfPistols = 0;
	pBlackBoard->GetData("WantedAmmountOfPistols", wantedAmmountOfPistols);

#pragma endregion

	//Check if there are less than wanted ammount pistols in the inventory
	if (AmmountOfTypeInInventory(pBlackBoard, eItemType::PISTOL) < wantedAmmountOfPistols)
	{
		//Set the blackboard data
		pBlackBoard->ChangeData("NeededItemType", eItemType::PISTOL);
		return true;
	}

	//No pistols found
	return false;
}

inline bool IsNeededItemInMemory(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get inventory
	std::vector<ItemInfo> itemsInMemory = {};
	pBlackBoard->GetData("ItemsInMemory", itemsInMemory);

	//Get the needed item typ
	eItemType neededItemType = {};
	pBlackBoard->GetData("NeededItemType", neededItemType);

	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

#pragma endregion

	//Vector to store al items of neededType
	std::vector<ItemInfo> itemsOfNeededType = {};

	//Var to store closestneededitem
	ItemInfo closestNeededItem = {};

	//Get all items of the neededType
	for each (ItemInfo itemInMemory in itemsInMemory)
	{
		if (itemInMemory.Type == neededItemType)
		{
			itemsOfNeededType.push_back(itemInMemory);
		}
	}

	//There is an item found of the neededtype
	if (itemsOfNeededType.empty() == false)
	{
		//Get the closest item of the type we need
		closestNeededItem = itemsOfNeededType[0];

		for each (ItemInfo itemOfType in itemsOfNeededType)
		{
			if (Elite::Distance(agent.Position, itemOfType.Location) < Elite::Distance(agent.Position, closestNeededItem.Location))
			{
				closestNeededItem = itemOfType;
			}
		}

		//Update blackboard with this closestNeededItem
		pBlackBoard->ChangeData("ClosestNeededItem", closestNeededItem);

		//Set the item as the target
		pBlackBoard->ChangeData("Target", closestNeededItem.Location);

		printf("A needed item is found in the memory, Moving towards it\n");

		//A neededItem is found and set, return true
		return true;
	}
	//Clear neededItem
	pBlackBoard->ChangeData("ClosestNeededItem", closestNeededItem);

	//There is no item of the needed type, return false
	return false;
}
#pragma endregion

#pragma region Search
inline bool AreHousesInFOV(Blackboard *pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	std::vector<HouseInfo> housesInFOV = {};
	pBlackBoard->GetData("HousesInFOV", housesInFOV);

#pragma endregion

	if (housesInFOV.empty())
	{
		return false;
	}
	return true;
}

inline bool ShouldEnterHouse(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the houses in the FOV
	std::vector<HouseInfo> housesInFOV = {};
	pBlackBoard->GetData("HousesInFOV", housesInFOV);

	//Get the recently visited houses list
	std::vector<HouseInfo> recentlyVisitedHouses = {};
	pBlackBoard->GetData("RecentlyVisitedHouses", recentlyVisitedHouses);

	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

#pragma endregion

	//Var for nin-ignored houses
	std::vector<HouseInfo> nonIgnoredHouses = {};

	//Look if there is a non-recently visited house
	for each (HouseInfo houseInFOV in housesInFOV)
	{
		//Search for the current inFOVHouse in the recently visted houses list
		auto it = std::find_if(recentlyVisitedHouses.begin(), recentlyVisitedHouses.end(), [&](HouseInfo visitedHouse)
		{
			return visitedHouse.Center == houseInFOV.Center;
		});

		//If the house is not found
		if (it == recentlyVisitedHouses.end())
		{
			nonIgnoredHouses.push_back(houseInFOV);
		}
	}

	//If there are non-visitedHouses in the FOV
	if (nonIgnoredHouses.empty() == false)
	{
		//Var for the closest house
		HouseInfo closestHouse = nonIgnoredHouses[0];

		//Get the closestone
		for each (HouseInfo nonIgnoredHouse in nonIgnoredHouses)
		{
			if (Elite::Distance(agent.Position, nonIgnoredHouse.Center) < Elite::Distance(agent.Position, closestHouse.Center))
			{
				closestHouse = nonIgnoredHouse;
			}
		}
		//Set the house as the new target
		pBlackBoard->ChangeData("Target", closestHouse.Center);

		//Set the house as the currentHouse
		pBlackBoard->ChangeData("CurrentHouse", closestHouse);

		//Set goingToHouse
		pBlackBoard->ChangeData("GoingToHouse", true);
		printf("Agent Should enter the house\n");

		return true;
	}
	return false;
}

inline bool IsInHouse(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

#pragma endregion

	return agent.IsInHouse;
}

inline bool IsNotInHouse(Blackboard* pBlackBoard)
{
	return !IsInHouse(pBlackBoard);
}

inline bool ShouldSearch(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the houses in the FOV
	std::vector<HouseInfo> housesInFOV = {};
	pBlackBoard->GetData("HousesInFOV", housesInFOV);

	//Get the recently visited houses list
	std::vector<HouseInfo> recentlyVisitedHouses = {};
	pBlackBoard->GetData("RecentlyVisitedHouses", recentlyVisitedHouses);

#pragma endregion

	//Look if there is a non-recently visited house
	for each (HouseInfo houseInFOV in housesInFOV)
	{
		//Search for the current inFOVHouse in the recently visted houses list
		auto it = std::find_if(recentlyVisitedHouses.begin(), recentlyVisitedHouses.end(), [&](HouseInfo visitedHouse)
		{
			return visitedHouse.Center == houseInFOV.Center;
		});

		//If the house is not found
		if (it == recentlyVisitedHouses.end())
		{
			return true;
		}
	}
	return false;
}

inline bool GoingToHouse(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get going to house
	bool goingToHouse = false;
	pBlackBoard->GetData("GoingToHouse", goingToHouse);

	//GEt the targetHouse
	HouseInfo targetHouse = {};
	pBlackBoard->GetData("CurrentHouse", targetHouse);

	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

#pragma endregion

	if (agent.IsInHouse || goingToHouse == false)
	{
		//Clear goingto house since we are in a house
		pBlackBoard->ChangeData("GoingToHouse", false);

		return false;
	}

	if (goingToHouse)
	{
		pBlackBoard->ChangeData("Target", targetHouse.Center);
		printf("Going To a House\n");
	}
	return true;
}
#pragma endregion

#pragma region PickUpOrMemoriseItems
inline bool AreItemsInFOV(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get al entities in the FOV
	std::vector<EntityInfo> entitiesInFOV = {};
	pBlackBoard->GetData("EntitiesInFOV", entitiesInFOV);

	//Get agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

#pragma endregion

	//Vector to store all items in FOV
	std::vector<EntityInfo> itemsInFOV = {};

	//Var to store the cloest item
	EntityInfo closestItem = {};

	//Add itemIntities to itemsInFOV
	for each (EntityInfo entityInFOV in entitiesInFOV)
	{
		if (entityInFOV.Type == eEntityType::ITEM)
		{
			itemsInFOV.push_back(entityInFOV);
		}
	}

	//Check if there are items
	if (itemsInFOV.empty() == false)
	{
		//Change the blackboard data for ItemsInFOV 
		pBlackBoard->ChangeData("ItemsInFOV", itemsInFOV);

		//Get the ClosestItem
		closestItem = itemsInFOV[0];
		
		for each (EntityInfo itemInFOV in itemsInFOV)
		{
			if (Elite::Distance(agent.Position, itemInFOV.Location) < Elite::Distance(agent.Position, closestItem.Location))
			{
				closestItem = itemInFOV;
			}
		}

		//Add the closestItem data to the blackboard
		pBlackBoard->ChangeData("ClosestItem", closestItem);

		//items found == return true
		return true;
	}

	//No items found == return false
	return false;
}

inline bool ItemIsInMemory(Blackboard *pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the memory
	std::vector<ItemInfo> itemsInMemory = {};
	pBlackBoard->GetData("ItemsInMemory", itemsInMemory);

	//Get the closest item
	EntityInfo closestItem = {};
	pBlackBoard->GetData("ClosestItem", closestItem);

#pragma endregion

	//Is there an item with the same location in the memory
	auto it = std::find_if(itemsInMemory.begin(), itemsInMemory.end(), [&](ItemInfo itemInMemory)
	{
		return itemInMemory.Location == closestItem.Location;
	});

	//If there is a item found
	if (it != itemsInMemory.end())
	{
		//Item is found, return true
		return true;
	}
	//Item is not found, return false
	return false;
}

inline bool ItemNotInMemory(Blackboard *pBlackBoard)
{
	if (ItemIsInMemory(pBlackBoard))
	{
		return false;
	}

	return true;
}

inline bool NeedItem(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the closestNeededItem
	ItemInfo closestNeededItem = {};
	pBlackBoard->GetData("ClosestNeededItem", closestNeededItem);

	//Get the closestItem
	EntityInfo closestItem = {};
	pBlackBoard->GetData("ClosestItem", closestItem);
#pragma endregion

	//Check if the closestItem and the closestNeededItem are the same
	if (closestNeededItem.Location == closestItem.Location)
	{
		return true;
	}

	return false;
}

inline bool IsItemInRange(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the closestNeededItem
	EntityInfo closestItem = {};
	pBlackBoard->GetData("ClosestItem", closestItem);

	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

#pragma endregion

	//Check if the agent is in grabrange of the item
	if (Elite::Distance(agent.Position, closestItem.Location) <= agent.GrabRange)
	{
		//Agent is in range, return true
		return true;
	}
	//Agent is not in grabrange, return false
	return false;
}

inline bool IsFreeSlotInInventory(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the inventory
	std::vector<ItemInfo> inventory = {};
	pBlackBoard->GetData("Inventory", inventory);

#pragma endregion
	//Empty item
	ItemInfo emptyItem = {};

	//Search a free slot 
	auto it = std::find_if(inventory.begin(), inventory.end(), [&](ItemInfo itemInInventory)
	{
		return itemInInventory.Location == emptyItem.Location;
	});

	//If there is a free slot
	if (it != inventory.end())
	{
		//Determine the index of the free slot
		int index = std::distance(inventory.begin(), it);
		pBlackBoard->ChangeData("FreeSlot", index);

		//A free slot is found, return true
		return true;
	}

	//There is no free slot, return false
	return false;
}

inline bool IsNeededItemInFOV(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get items in FOV
	std::vector<EntityInfo> itemsInFOV = {};
	pBlackBoard->GetData("ItemsInFOV", itemsInFOV);

	//Get the neededItem
	ItemInfo closestNeededItem = {};
	pBlackBoard->GetData("ClosestNeededItem", closestNeededItem);
#pragma endregion

	//Search for the needed item in the FOV
	auto it = std::find_if(itemsInFOV.begin(), itemsInFOV.end(), [&](EntityInfo itemInFOV) 
	{
		return itemInFOV.Location == closestNeededItem.Location;
	});

	//If the item is found
	if (it != itemsInFOV.end())
	{
		//Set the closestNeededItem as the closestItem
		pBlackBoard->ChangeData("Target", closestNeededItem.Location);

		printf("Needed Item is in FOV, Moving to it");

		//new target is set, return true
		return true;
	}
	//the neededitem is not in the FOV, return false
	return false;
}
#pragma endregion

inline bool HasGarbageInInventory(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the inventory
	std::vector<ItemInfo> inventory = {};
	pBlackBoard->GetData("Inventory", inventory);

#pragma endregion

	//Search for garbage i hte inventory
	auto it = std::find_if(inventory.begin(), inventory.end(), [](ItemInfo itemInInventory)
	{
		return itemInInventory.Type == eItemType::GARBAGE;
	});

	//Is there garbage found?
	if (it != inventory.end())
	{
		return true;
	}
	return false;
}

inline bool ShouldRemoveFromIgnoreList(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get deltaTime
	float deltaTime = 0.0f;
	pBlackBoard->GetData("DeltaTime", deltaTime);

	//Get time till revisit
	float timeTillRevisit = 180.0f;
	pBlackBoard->GetData("TimerTillRevisit", timeTillRevisit);

	//Get currentTimer
	float revisitCounter = 0.0f;
	pBlackBoard->GetData("RevisitCounter", revisitCounter);

	//Get ignore list
	std::vector<HouseInfo> recenltyVisitedHouses = {};
	pBlackBoard->GetData("RecentlyVisitedHouses", recenltyVisitedHouses);
#pragma endregion

	//Check if there are houses to be ignored
	if (recenltyVisitedHouses.empty() == false)
	{
		//Add deltaTime to counter
		revisitCounter += deltaTime;

		//Check if counter passes the max Tiem
		if (revisitCounter >= timeTillRevisit)
		{
			//Update the blackboard
			pBlackBoard->ChangeData("RevisitCounter", 0.0f);

			//House can be visited again so remove it, return true
			return true;
		}

		//Update the blackboard
		pBlackBoard->ChangeData("RevisitCounter", revisitCounter);
	}

	//No house visited OR not yet time to revisited the house
	return false;
}

#pragma endregion

#pragma region Actions
inline BehaviorState MoveToCheckPoint(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the interface
	IExamInterface* pInterface = nullptr;
	pBlackBoard->GetData("Interface", pInterface);
#pragma endregion

	pBlackBoard->ChangeData("Target", pInterface->World_GetCheckpointLocation());

	return Seek(pBlackBoard);
}

inline BehaviorState RestoreHealthAndEnergy(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//GEt Agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

	//Get the max health/energy value
	float maxValue = 10;
	pBlackBoard->GetData("MaxHealthAndEnergyValue", maxValue);
#pragma endregion

	BehaviorState returnvalue = Failure;

	returnvalue = UseRestoreItem(pBlackBoard, eItemType::MEDKIT, "health", agent.Health);
	returnvalue = UseRestoreItem(pBlackBoard, eItemType::FOOD, "energy", agent.Energy);

	return returnvalue;
}

inline BehaviorState AimAtEnemy(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//GEt Agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

	//Get the closest Enemy
	EnemyInfo enemy = {};
	pBlackBoard->GetData("ClosestEnemy", enemy);

	//Get Shooting Accuracy
	float shootingAccuracy = 0.0f;
	pBlackBoard->GetData("ShootingAccuracy", shootingAccuracy);

#pragma endregion

	//Disable auto orientation
	pBlackBoard->ChangeData("AutoOrientate", false);

	//Set up a roationForward
	Elite::Vector2 rotationForward = { 0, -1 };

	//Get the orientation
	float orientation = agent.Orientation;

	//Rotate theis vector to match the orientation
	rotationForward = {		rotationForward.x * cos(orientation) - rotationForward.y * sin(orientation),
										rotationForward.x * sin(orientation) + rotationForward.y * cos(orientation) };

	//Get the direction vector to the eenmy
	Elite::Vector2 agentPosToEnemyLoc = enemy.Location - agent.Position;

	//Normalize all vectors
	Elite::Normalize(rotationForward);
	Elite::Normalize(agentPosToEnemyLoc);

	//what is the angle between the 2?
	auto angle = atan2(rotationForward.x*agentPosToEnemyLoc.y - rotationForward.y*agentPosToEnemyLoc.x,
									rotationForward.x*agentPosToEnemyLoc.x + rotationForward.y*agentPosToEnemyLoc.y);

	//If the angle is negative tell the steering to roate counter-clockwise
	if (angle < 0)
	{
		printf("Aiming rotate counter-ClockWise");
		//Add a negative rotation value to the blackboard
		pBlackBoard->ChangeData("RotationValue", agent.MaxAngularSpeed *-1);
	}
	else if(angle > 0)
	{
		printf("Aiming rotate ClockWise");
		//Add positive rotation value to the blackboard
		pBlackBoard->ChangeData("RotationValue", agent.MaxAngularSpeed);
	}

	//If the angle is in acceptable range we return succes
	if (abs(angle) < (_Pi / 180 * shootingAccuracy))
	{
		printf("Right into the sniper scope!\n");
		return Success;
	}
	printf("Not in the sniper scope\n");
	return Failure;
}

inline BehaviorState Shoot(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get bestPsitol
	int bestPistolIndex = {};
	pBlackBoard->GetData("BestPistolIndex", bestPistolIndex);

	//GEt interface
	IExamInterface* pInterface = nullptr;
	pBlackBoard->GetData("Interface", pInterface);

	//Get the inventory
	std::vector<ItemInfo> inventory = {};
	pBlackBoard->GetData("Inventory", inventory);

#pragma endregion
	
	//Use the bestGun
	pInterface->Inventory_UseItem(bestPistolIndex);
	printf("Pang Pang");

	//Get the bestGun
	ItemInfo bestPistol = {};
	pInterface->Inventory_GetItem(bestPistolIndex, bestPistol);

	//Get ammoCount
	int ammoCount = pInterface->Item_GetMetadata(bestPistol, "ammo");

	//If there is no more ammo
	if (ammoCount <= 0)
	{
		//Remove pistol
		pInterface->Inventory_RemoveItem(bestPistolIndex);

		//Remove pistol from personal inventory
		ItemInfo emptyItem = {};
		inventory[bestPistolIndex] = emptyItem;
		pBlackBoard->ChangeData("Inventory", inventory);


		//Clear the index
		bestPistolIndex = -1;
		pBlackBoard->ChangeData("BestPistolIndex", bestPistolIndex);

		printf("Rip no more ammo in this gun\n");
	}
	return Success;
}

inline BehaviorState SearchHouse(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the currentHouse
	HouseInfo currentHouse = {};
	pBlackBoard->GetData("CurrentHouse", currentHouse);

	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

	//Get if we enteredTheHouse
	bool hasEnteredHouse = false;
	pBlackBoard->GetData("HasEnteredHouse", hasEnteredHouse);

	//Get the currentTarget
	Elite::Vector2 currentTarget = {};
	pBlackBoard->GetData("Target", currentTarget);

	//Get the orderOfCorner visits
	std::vector<Elite::Vector2> orderOfCornerVisits = {};
	pBlackBoard->GetData("OrderOfCornerVisits", orderOfCornerVisits);

	//Get the list off recentlyVisitedHouses
	std::vector<HouseInfo> recentlyVisitedHouses = {};
	pBlackBoard->GetData("RecentlyVisitedHouses", recentlyVisitedHouses);

	//Get the targetCorner
	Elite::Vector2 targetCorner = {};
	pBlackBoard->GetData("TargetCorner", targetCorner);
#pragma endregion

	//If we just entered the house calculate order of corners
	if (hasEnteredHouse == false)
	{
		//Calculate all corners
		std::vector<Elite::Vector2> corners = {};
		Elite::Vector2 halfSize = { currentHouse.Size.x / 2, currentHouse.Size.y / 2 };
		Elite::Vector2 offset = { 5, 5 };

		corners.push_back({ currentHouse.Center.x - halfSize.x + offset.x, currentHouse.Center.y + halfSize.y - offset.y });
		corners.push_back({ currentHouse.Center.x + halfSize.x - offset.x, currentHouse.Center.y + halfSize.y - offset.y });
		corners.push_back({ currentHouse.Center.x - halfSize.x + offset.x, currentHouse.Center.y - halfSize.y + offset.y });
		corners.push_back({ currentHouse.Center.x + halfSize.x - offset.x, currentHouse.Center.y - halfSize.y + offset.y });

		//Clear && Put the corners in the order we want to visit 
		orderOfCornerVisits = {};

		//First corner to visit == closest when we eneter
		orderOfCornerVisits.push_back(GetClosestCorner(agent.Position, corners));
		//Second corner to visit == furthes corner to the first corner
		orderOfCornerVisits.push_back(GetFurthestCorner(orderOfCornerVisits[0], corners));
		//Third corner to visit == closest corner to the second corner
		orderOfCornerVisits.push_back(GetClosestCorner(orderOfCornerVisits[1], corners));
		//Last corner to visit == furhtest corner to the third corner
		orderOfCornerVisits.push_back(GetFurthestCorner(orderOfCornerVisits[2], corners));

		//Save this order so we don't have to recalculate them
		pBlackBoard->ChangeData("OrderOfCornerVisits", orderOfCornerVisits);

		//Tell the blackboard that all needed information is set
		pBlackBoard->ChangeData("HasEnteredHouse", true);

		//Set the target to the first corner
		pBlackBoard->ChangeData("Target", orderOfCornerVisits[0]);

		//Set the target corner
		pBlackBoard->ChangeData("TargetCorner", orderOfCornerVisits[0]);
	}
	else
	{
		//Check if the agent is going to a corner
		auto it2 = std::find_if(orderOfCornerVisits.begin(), orderOfCornerVisits.end(), [&](Elite::Vector2 cornerPosition)
		{
			return cornerPosition == currentTarget;
		});

		//If the target is not one of the corners
		if (it2 == orderOfCornerVisits.end())
		{
			//Set the target to the target corner
			pBlackBoard->ChangeData("Target", targetCorner);
		}
		else
		{
		//If the agent is close to the target change the target
			if (Elite::Distance(agent.Position, currentTarget) < 3.0f)
			{
				//Find the currentTarget in the order
				auto it = std::find_if(orderOfCornerVisits.begin(), orderOfCornerVisits.end(), [&](Elite::Vector2 cornerPosition)
				{
					return cornerPosition == currentTarget;
				});

				//Check if it is valid
				if (it != orderOfCornerVisits.end())
				{
					//Determine the index
					int index = std::distance(orderOfCornerVisits.begin(), it);

					//Increment the index
					index++;

					//Check if there is a next corner
					if (index < orderOfCornerVisits.size())
					{
						//Set the target to the next corner
						pBlackBoard->ChangeData("Target", orderOfCornerVisits[index]);

						//Set a new targetCorner
						pBlackBoard->ChangeData("TargetCorner", orderOfCornerVisits[index]);

					}
					else
					{
						//Add house to recentlyVisitedList
						recentlyVisitedHouses.push_back(currentHouse);
						pBlackBoard->ChangeData("RecentlyVisitedHouses", recentlyVisitedHouses);

						//Reset
						bool hasEnteredHouse = false;
						pBlackBoard->ChangeData("HasEnteredHouse", hasEnteredHouse);

						std::vector<Elite::Vector2> orderOfCornerVisits = {};
						pBlackBoard->ChangeData("OrderOfCornerVisits", orderOfCornerVisits);

						Elite::Vector2 emptyCorner = {0,0};
						pBlackBoard->ChangeData("TargetCorner", emptyCorner);

						return Failure;
					}
				}
			}
		}
	}
	printf("Searching\n");
	pBlackBoard->ChangeData("CanRun", true);
	return Seek(pBlackBoard);
}

inline BehaviorState RemoveGarbage(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the interface
	IExamInterface* pInterface = nullptr;
	pBlackBoard->GetData("Interface", pInterface);

	//Get the inventory
	std::vector<ItemInfo> inventory = {};
	pBlackBoard->GetData("Inventory", inventory);

#pragma endregion

	//Search for the garbage in the inventory
	auto it = std::find_if(inventory.begin(), inventory.end(), [](ItemInfo itemInInventory)
	{
		return itemInInventory.Type == eItemType::GARBAGE;
	});

	//If there is garbage found
	if (it != inventory.end())
	{
		//Get the place of the garbage
		int index = std::distance(inventory.begin(), it);

		//remove the garbage from the inventory
		pInterface->Inventory_RemoveItem(index);

		//Create an empty item
		ItemInfo emptyItem = {};

		//Remove the garbage fro  the personal inventory
		inventory[index] = emptyItem;

		//Update the blackboard with the intventory
		pBlackBoard->ChangeData("Inventory", inventory);

		return Success;
	}
	return Failure;
}

inline BehaviorState AddItemToInventory(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the free slot
	int freeSlotIndex = 0;
	pBlackBoard->GetData("FreeSlot", freeSlotIndex);

	//Get the interface
	IExamInterface* pInterface = {};
	pBlackBoard->GetData("Interface", pInterface);

	//Get the inventory
	std::vector<ItemInfo> inventory = {};
	pBlackBoard->GetData("Inventory", inventory);

	//Get the closestItem
	EntityInfo closestItem = {};
	pBlackBoard->GetData("ClosestItem", closestItem);

	//Get the memory
	std::vector<ItemInfo> itemsInMemory = {};
	pBlackBoard->GetData("ItemsInMemory", itemsInMemory);

#pragma endregion

	//Get the ItemInfo of the closestItem
	ItemInfo closestItemConverted = {};

	if (pInterface->Item_Grab(closestItem, closestItemConverted))
	{
		//Add item to interface inventory
		if (pInterface->Inventory_AddItem(freeSlotIndex, closestItemConverted))
		{
			//Add the closestItem to the personal inventory
			inventory[freeSlotIndex] = closestItemConverted;
			pBlackBoard->ChangeData("Inventory", inventory);

			if (itemsInMemory.empty() == false)
			{
				//Remove the item from the memory if it is in there
				itemsInMemory.erase(std::remove_if(itemsInMemory.begin(), itemsInMemory.end(), [&](ItemInfo itemInMemory)
				{
					return itemInMemory.Location == closestItemConverted.Location;
				}), itemsInMemory.end());

				pBlackBoard->ChangeData("ItemsInMemory", itemsInMemory);
			}
			//Item is added to the inventory, return Succes
			return Success;
		}
	}

	//Could not add the item to the inventory, return failure
	return Failure;
}

inline BehaviorState MoveToItem(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the closestItem
	EntityInfo closestItem = {};
	pBlackBoard->GetData("ClosestItem", closestItem);

#pragma endregion

	//Set the target to the closest item
	pBlackBoard->ChangeData("Target", closestItem.Location);

	printf("MovingToClosest item\n");

	//Seek to the target
	return Seek(pBlackBoard);
}

inline BehaviorState AddItemToMemory(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get closestItem
	EntityInfo closestItem = {};
	pBlackBoard->GetData("ClosestItem", closestItem);

	//Get memory
	std::vector<ItemInfo> itemsInMemory = {};
	pBlackBoard->GetData("ItemsInMemory", itemsInMemory);

	//Get the interface
	IExamInterface* pInterface = nullptr;
	pBlackBoard->GetData("Interface", pInterface);

#pragma endregion

	//Get the ItemInfo of the closestItem
	ItemInfo closestItemConverted = {};

	if (pInterface->Item_Grab(closestItem, closestItemConverted))
	{
		//Add the item to the memory
		itemsInMemory.push_back(closestItemConverted);

		//Update the blackboard
		pBlackBoard->ChangeData("ItemsInMemory", itemsInMemory);

		//Print the type
		switch (closestItemConverted.Type)
		{
		case eItemType::FOOD:
			printf("Added FOOD to memory\n");
			break;
		case eItemType::MEDKIT:
			printf("Added MEDKIT to memory\n");
			break;
		case eItemType::PISTOL:
			printf("Added PSITOL to memory\n");
			break;
		default:
			break;
		}

		//Item is added to the Memory, return Succes
		return Success;
	}

	//Item could not be converted return failure
	return Failure;
}

inline BehaviorState DropUnNeededItem(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get wanted food ammount
	int wantedAmmountOfFood = 0;
	pBlackBoard->GetData("WantedAmmountOfFood", wantedAmmountOfFood);

	//Get wanted medkit ammount
	int wantedAmmountOfMedkits = 0;
	pBlackBoard->GetData("WantedAmmountOfMedkits", wantedAmmountOfMedkits);

	//Get wanted ammount of pistols
	int wantedAmmountOfPistols = 0;
	pBlackBoard->GetData("WantedAmmountOfPistols", wantedAmmountOfPistols);

#pragma endregion

	//Check if we have more pistol than we need
	if (AmmountOfTypeInInventory(pBlackBoard, eItemType::PISTOL) > wantedAmmountOfPistols)
	{
		if (ReemoveItem(pBlackBoard, eItemType::PISTOL))
		{
			printf("Removed a gun\n");
			return Success;
		}
		else
		{
			return Failure;
		}
	}
	else if (AmmountOfTypeInInventory(pBlackBoard, eItemType::MEDKIT) > wantedAmmountOfMedkits)
	{
		if (ReemoveItem(pBlackBoard, eItemType::MEDKIT))
		{
			printf("Removed a medkit\n");
			return Success;
		}
		else
		{
			return Failure;
		}
	}
	else if (AmmountOfTypeInInventory(pBlackBoard, eItemType::FOOD) > wantedAmmountOfFood)
	{
		if (ReemoveItem(pBlackBoard, eItemType::FOOD))
		{
			printf("Removed food\n");
			return Success;
		}
		else
		{
			return Failure;
		}
	}

	return Failure;
}

inline BehaviorState RemoveFirstFromIgnoreList(Blackboard* pBlackBoard)
{
#pragma region Get_Info_From_Blackboard
	//Get the ignore =List
	std::vector<HouseInfo> recentlyVisitedHouses = {};
	pBlackBoard->GetData("RecentlyVisitedHouses", recentlyVisitedHouses);

#pragma endregion

	if (recentlyVisitedHouses.empty() == false)
	{
		//Remove the fisrt element
		recentlyVisitedHouses.erase(recentlyVisitedHouses.begin());
		pBlackBoard->ChangeData("RecentlyVisitedHouses", recentlyVisitedHouses);

		return Success;
	}

	return Failure;
}
#pragma endregion
