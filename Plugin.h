#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "BehaviourTree/BehaviorTree.h"

class IBaseInterface;
class IExamInterface;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void ProcessEvents(const SDL_Event& e) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
#pragma region BASE
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;

	Elite::Vector2 m_Target = {};
#pragma endregion

	//BehaviorTree
	BehaviorTree *m_pBehaviourTree;
	Blackboard* CreateBlackBoard();
	void CreateBehaviorTree();
	void UpdateBlackBoard(float dt);

	void RegulateResting(float threshHold);
	void RegulateWantedMedkits();
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}