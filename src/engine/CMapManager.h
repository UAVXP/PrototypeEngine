#ifndef ENGINE_CMAPMANAGER_H
#define ENGINE_CMAPMANAGER_H

#include <SDL2/SDL.h>

#include "CCamera.h"

struct bmodel_t;
class CBaseEntity;

class CMapManager final
{
private:
	static const float ROTATE_SPEED;
	static const float MOVE_SPEED;

public:
	CMapManager() = default;
	~CMapManager() = default;

	bool LoadMap( const char* const pszMapName );

	void FreeMap();

	bool IsMapLoaded() const { return m_pModel != nullptr; }

	void RenderMap( long long uiDeltaTime );

	void HandleSDLEvent( SDL_Event& event );

private:
	void RenderModel( const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model, 
					  const CBaseEntity* pEntity, bmodel_t& brushModel, size_t& uiCount, size_t& uiTriangles, double& flTotal );

	void KeyEvent( const SDL_KeyboardEvent& event );

	void MouseButtonEvent( const SDL_MouseButtonEvent& event );

	void MouseMotionEvent( const SDL_MouseMotionEvent& event );

	void MouseWheelEvent( const SDL_MouseWheelEvent& event );

private:
	bmodel_t* m_pModel = nullptr;

	CCamera m_Camera;

	float m_flDeltaTime = 0;
	float m_flYawVel = 0;
	float m_flPitchVel = 0;

private:
	CMapManager( const CMapManager& ) = delete;
	CMapManager& operator=( const CMapManager& ) = delete;
};

#endif //ENGINE_CMAPMANAGER_H
