#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "BehaviourTree\Behaviors.h"

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);
	CreateBehaviorTree();

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Eobard Thawne";
	info.Student_FirstName = "Koen";
	info.Student_LastName = "Goossens";
	info.Student_Class = "2DAE4";
}

//Called only once
void Plugin::DllInit()
{
	//Can be used to figure out the source of a Memory Leak
	//Possible undefined behavior, you'll have to trace the source manually 
	//if you can't get the origin through _CrtSetBreakAlloc(0) [See CallStack]
	//_CrtSetBreakAlloc(0);

	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
	SAFE_DELETE(m_pBehaviourTree);
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
							//params.LevelFile = "LevelTwo.gppl";
	params.AutoGrabClosestItem = false; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.OverrideDifficulty = false; //Override Difficulty?
	params.Difficulty = 1.f; //Difficulty Override: 0 > 1 (Overshoot is possible, >1)
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::ProcessEvents(const SDL_Event& e)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	switch (e.type)
	{
	case SDL_MOUSEBUTTONUP:
		{
			if (e.button.button == SDL_BUTTON_LEFT)
			{
				int x, y;
				SDL_GetMouseState(&x, &y);
				const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(x), static_cast<float>(y));
				m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
			}
			break;
		}
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	UpdateBlackBoard(dt);
	m_pBehaviourTree->Update();

	auto steering = SteeringPlugin_Output();
	m_pBehaviourTree->GetBlackboard()->GetData("Steering", steering);

	m_pBehaviourTree->GetBlackboard()->GetData("Target", m_Target);

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

#pragma region BehaviorTree
Blackboard * Plugin::CreateBlackBoard()
{
	auto pBlackBoard = new Blackboard;
	
	std::vector<EntityInfo> entityVector = {};
	std::vector<ItemInfo> itemVector = {};
	std::vector<EnemyInfo> enemyVector = {};
	std::vector<HouseInfo> houseVector = {};

	pBlackBoard->AddData("Interface", m_pInterface);
	pBlackBoard->AddData("Agent", m_pInterface->Agent_GetInfo());
	pBlackBoard->AddData("EntitiesInFOV", entityVector);

#pragma region Steering
	pBlackBoard->AddData("Target", m_Target);

	SteeringPlugin_Output steering = {};
	pBlackBoard->AddData("Steering", steering);

	pBlackBoard->AddData("AutoOrientate", true);
	pBlackBoard->AddData("IsResting", false);
	pBlackBoard->AddData("CanRun", false);

	float rotationValue = 0.0f;
	pBlackBoard->AddData("RotationValue", rotationValue);
#pragma endregion

#pragma region Shooting/Evading
	EnemyInfo emptyEnemy = {};
	pBlackBoard->AddData("ClosestEnemy", emptyEnemy);

	int bestPistolIndex = -1;
	pBlackBoard->AddData("BestPistolIndex", bestPistolIndex);

	float minEvadeAngleInDegree = 10;
	pBlackBoard->AddData("MinEvadeAngle", minEvadeAngleInDegree);

	float shootingAccuracy = 2.0f;
	pBlackBoard->AddData("ShootingAccuracy", shootingAccuracy);
#pragma endregion

#pragma region ItemUsage
	float maxHealthAndEnergyValue = 10.0f;
	pBlackBoard->AddData("MaxHealthAndEnergyValue", maxHealthAndEnergyValue);
#pragma endregion

#pragma region Items
	pBlackBoard->AddData("ItemsInMemory", itemVector);
	pBlackBoard->AddData("ItemsInFOV", entityVector);

	EntityInfo closestItem = {};
	pBlackBoard->AddData("ClosestItem", closestItem);

#pragma region inventory
	UINT inventoryCapacity = m_pInterface->Inventory_GetCapacity();
	std::vector<ItemInfo> inventory = {};
	ItemInfo emptyItem = {};
	for (UINT i = 0; i < inventoryCapacity; i++)
	{
		inventory.push_back(emptyItem);
	}
	pBlackBoard->AddData("Inventory", inventory);

	int freeSlot = -1;
	pBlackBoard->AddData("FreeSlot", freeSlot);

#pragma endregion

#pragma endregion

#pragma region SpecificItemSearch
	int wantedAmmountOfFood = 2;
	pBlackBoard->AddData("WantedAmmountOfFood", wantedAmmountOfFood);

	int wantedAmmountOfMedkits = 1;
	pBlackBoard->AddData("WantedAmmountOfMedkits", wantedAmmountOfMedkits);

	int wantedAmmountOfPistols = 1;
	pBlackBoard->AddData("WantedAmmountOfPistols", wantedAmmountOfPistols);

	pBlackBoard->AddData("NeededItemType", eItemType::PISTOL);

	ItemInfo closestNeededItem = {};
	pBlackBoard->AddData("ClosestNeededItem", closestNeededItem);
#pragma endregion
	
#pragma region Search
	pBlackBoard->AddData("HousesInFOV", houseVector);
	pBlackBoard->AddData("RecentlyVisitedHouses", houseVector);

	HouseInfo currentHouse = {};
	pBlackBoard->AddData("CurrentHouse", currentHouse);

	std::vector<Elite::Vector2> orderOfCornerVisits = {};
	pBlackBoard->AddData("OrderOfCornerVisits", orderOfCornerVisits);

	bool hasEnteredHouse = false;
	pBlackBoard->AddData("HasEnteredHouse", hasEnteredHouse);

	Elite::Vector2 targetCorner = {};
	pBlackBoard->AddData("TargetCorner", targetCorner);

	float deltaTime = 0.0f;
	pBlackBoard->AddData("DeltaTime", deltaTime);

	float timeTillRevisit = 60.0f;
	pBlackBoard->AddData("TimerTillRevisit", timeTillRevisit);

	float revisitedCounter = 0.0f;
	pBlackBoard->AddData("RevisitCounter", revisitedCounter);

	bool goingToHouse = false;
	pBlackBoard->AddData("GoingToHouse", goingToHouse);
#pragma endregion

	return pBlackBoard;
}

void Plugin::CreateBehaviorTree()
{
	m_pBehaviourTree = new BehaviorTree(CreateBlackBoard(), new BehaviorSelector(
		{
			new BehaviorSequence(	//Use/remove Items if needed SEQUENCE
				{
					new BehaviorSelector(		//Remove garbage OR use item OR Enable revisiting a house SELECTOR
						{
							new BehaviorSequence(	//Remove garbage SEQUENCE
								{
									new BehaviorConditional(HasGarbageInInventory),	//Is there garbage in the inventory?
									new BehaviorAction(RemoveGarbage)	//Remove the garbage
								}),
							new BehaviorSequence(	//Use item SEQUENCE
								{
									new BehaviorAction(RestoreHealthAndEnergy)	//Can we use an item
								}),
							new BehaviorSequence(
								{
									new BehaviorConditional(ShouldRemoveFromIgnoreList),
									new BehaviorAction(RemoveFirstFromIgnoreList)
								})
						})
				}),
			new BehaviorSequence(	//What to do when there are enemies SEQUENCE
				{
					new BehaviorConditional(AreEnemiesInFOV), //Are there enemies in the FOV
					new BehaviorSelector( //Should we evade or shoot/evade while inside or outside a house SELECTOR
						{
							new BehaviorSelector( //Are we in a house and is the enemy in the FOV also in that house or none of both SELECTOR
								{
									new BehaviorSequence(	//In a house and enemy too SEQUENCE
										{
											new BehaviorConditional(IsInHouse),	//are we in a house?
											new BehaviorConditional(IsEnemyInHouse),	//Is the nemy also in this house?
											new BehaviorSelector(	//Do we have a gun or not SELECTOR
												{
													new BehaviorSequence(	//Shoot/evade SEQUENCE
														{
															new BehaviorConditional(CanShootAtEnemy),	//Do we have a gun to use? && a target?
															//new BehaviorAction(Evade),	//Evade the closest enemy
															new BehaviorAction(AimAtEnemy), //turn to aim at the enemy
															new BehaviorAction(Shoot)		//Shoot at the closestEnemy
														}),
													new BehaviorSequence(	//Evade SEQUENCE
														{
															new BehaviorAction(Evade),	//Evade the closest enemy
														})
												})
										}),
									new BehaviorSequence(	//We are not in a house SEQUENCE
										{
											new BehaviorConditional(IsNotInHouse),	//check if we are not in a house
											new BehaviorSelector(	//Do we have a gun or not SELECTOR
												{
													new BehaviorSequence(	//Shoot/evade SEQUENCE
														{
															new BehaviorConditional(CanShootAtEnemy),	//Do we have a gun to use? && a target?
															//new BehaviorAction(Evade),	//Evade the closest enemy
															new BehaviorAction(AimAtEnemy), //turn to aim at the enemy
															new BehaviorAction(Shoot)		//Shoot at the closestEnemy
														}),
													new BehaviorSequence(	//Evade SEQUENCE
														{
															new BehaviorAction(Evade),	//Evade the closest enemy
														})
												})
										})
								})
						})				
				}),
			new BehaviorSequence(	//Items are in the FOV SEQUENCE
				{
					new BehaviorConditional(AreItemsInFOV),	 //Check if there are items in the FOV? && set the closest item
					new BehaviorSelector(	//Is the closestItem in the memory SELECTOR
						{
							new BehaviorSequence(	//ClosestItem is in memory SEQUENCE
								{
									new BehaviorConditional(ItemIsInMemory),	//Is the item in the memory?
									new BehaviorSelector(		//Is the item needed SELCTOR
										{
											new BehaviorSequence(	//The item is needed SEQUENCE
												{
													new BehaviorConditional(NeedItem),	//Is the item someting we need?
													new BehaviorSelector(	//Is the item in range SELECTOR
														{
															new BehaviorSequence(	//The item is in range SEQUENCE
																{
																	new BehaviorConditional(IsItemInRange),	//Is the item in range?
																	new BehaviorSelector(	//Is there a free slot in the inventory SELECTOR
																		{
																			new BehaviorSequence(	//There is a free slot in the inventory SEQUENCE
																				{
																					new BehaviorConditional(IsFreeSlotInInventory),	//Is there a free slot?
																					new BehaviorAction(AddItemToInventory)	//Add item to inventory
																				}),
																			new BehaviorSequence(	//There is no free slot in the inventory SEQUENCE
																				{
																					new BehaviorAction(DropUnNeededItem)	//Drop a unneeded item
																				}),
																		})
																}),
															new BehaviorSequence(	//Not in range of the needed item SEQUENCE
																{
																	new BehaviorAction(MoveToItem)	//Move to the closest item == neededItem
																})
														})
												}),
											new BehaviorSequence(	//The closest Item is not the needed item SEQUENCE
												{
													new BehaviorConditional(IsNeededItemInFOV),	//Is the needed item in the FOV? set the target
													new BehaviorAction(Seek)	//Move to the needed item (target is set)
												})
										})
								}),
							new BehaviorSequence(	//ClosestItem is not in memory
								{
									new BehaviorConditional(ItemNotInMemory),	//Is the item NOT in the memory?
									new BehaviorSelector( //Is the item in range SELECTOR
										{
											new BehaviorSequence(	//Item is in range SEQUENCE
												{
													new BehaviorConditional(IsItemInRange),	//Check if item is in range?
													new BehaviorSelector(	//Is there a free slot SELECTOR
														{
															new BehaviorSequence(	//There is a free slot EQUENCE
																{
																	new BehaviorConditional(IsFreeSlotInInventory),	//Is there a free slot?
																	new BehaviorAction(AddItemToInventory)	//Add item to inventory
																}),
														new BehaviorSequence(	//There is no free slot sequence
															{
																new BehaviorAction(AddItemToMemory)	//Add item to memory
															})
														})
												}),
											new BehaviorSequence(	//Item is not in range SEQUENCE
												{
													new BehaviorAction(MoveToItem)	//Move tot he clsoest item
												})
										}),
								})
						}),
				}),
			new BehaviorSequence(	//Search house SEQUENCE
			{
				new BehaviorSelector(
					{
						new BehaviorSequence(
							{
								new BehaviorConditional(GoingToHouse),
								new BehaviorAction(Seek)
							}),
						new BehaviorSequence(
							{
								new BehaviorSelector(	//Are we searching OR did we just see a house SELECTOR
									{
										new BehaviorSequence(	//Search a house SEQUENCE
											{
												new BehaviorConditional(IsInHouse),	//Check if we are in a house?
												new BehaviorConditional(ShouldSearch),	//Check if we are doing a search
												new BehaviorAction(SearchHouse)	//Search the house
											}),
									new BehaviorSequence(	//Decide if we want to visted the house SEQUENCE
										{
											new BehaviorConditional(AreHousesInFOV),	//Are there houses in the FOV?
											new BehaviorConditional(ShouldEnterHouse),	//Is there a not ignored or recently visited house in the FOV?
											new BehaviorAction(Seek)	//Move to that house
										})
									})
							})
					})
			}),
			new BehaviorSequence(	//What to do if we need a item && dont have it SEQUENCE
				{
					new BehaviorSelector(	//Wich item do we need to find? SELECTOR
						{
							new BehaviorSequence(	//agent needs health SEQUENCE
								{
									new BehaviorConditional(LowOnHealth),	//Is the agent low on health?
									new BehaviorConditional(IsNeededItemInMemory),	//Is there a medkit in the memory?
									new BehaviorAction(Seek)	//Go to the medkit
								}),
							new BehaviorSequence(	//agent needs Energy SEQUENCE
								{
									new BehaviorConditional(LowOnEnergy),	//Is the agent low on energy
									new BehaviorConditional(IsNeededItemInMemory),	//Is there food in the memory
									new BehaviorAction(Seek)	//Go to food
								}),
						new BehaviorSequence(	//agent needs a pistol SEQUENCE
							{
								new BehaviorConditional(LowOnPistols),	//Is the agent low on pistols?
								new BehaviorConditional(IsNeededItemInMemory),	//Is there Pistol in the memory
								new BehaviorAction(Seek)	//Go to Pistol
							})
						})
				}),
			new BehaviorAction(MoveToCheckPoint) //FALLBACK
		}));
}
void Plugin::UpdateBlackBoard(float dt)
{
	auto pBlackBoard = m_pBehaviourTree->GetBlackboard();

	pBlackBoard->ChangeData("Agent", m_pInterface->Agent_GetInfo());
	pBlackBoard->ChangeData("EntitiesInFOV", GetEntitiesInFOV());
	pBlackBoard->ChangeData("HousesInFOV", GetHousesInFOV());
	//->ChangeData("Interface", m_pInterface);
	pBlackBoard->ChangeData("DeltaTime", dt);

	pBlackBoard->ChangeData("CanRun", false);
	RegulateResting(5.0f);
	RegulateWantedMedkits();
}

void Plugin::RegulateResting(float threshHold)
{
	//Get the blackboard
	auto pBlackBoard = m_pBehaviourTree->GetBlackboard();

	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

	if (agent.Stamina <= 0.0f)
	{
		pBlackBoard->ChangeData("IsResting", true);
	}

	if (agent.Stamina >= threshHold)
	{
		pBlackBoard->ChangeData("IsResting", false);
	}
}

void Plugin::RegulateWantedMedkits()
{
	//Get the blackboard
	auto pBlackBoard = m_pBehaviourTree->GetBlackboard();

	//Get the agent
	AgentInfo agent = {};
	pBlackBoard->GetData("Agent", agent);

	if (agent.Energy <= 0.0f)
	{
		pBlackBoard->ChangeData("WantedAmmountOfMedkits", 2);
	}
	else
	{
		pBlackBoard->ChangeData("WantedAmmountOfMedkits", 1);
	}
}
#pragma endregion
