#include <chrono>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Common.h"
#include "Logging.h"

#include "bsp/BSPIO.h"
#include "bsp/BSPRenderIO.h"
#include "entity/EntityIO.h"
#include "entity/CEntityList.h"
#include "entity/CBaseEntity.h"

#include "gl/GLUtil.h"
#include "gl/CShaderManager.h"
#include "gl/CShaderInstance.h"

#include "Engine.h"

#include "CMapManager.h"

const float CMapManager::ROTATE_SPEED = 120.0f;
const float CMapManager::MOVE_SPEED = 100.0f;

bool CMapManager::LoadMap( const char* const pszMapName )
{
	ASSERT( pszMapName );

	char szMapName[ MAX_PATH ];

	snprintf( szMapName, sizeof( szMapName ), "maps/%s.bsp", pszMapName );

	auto header = LoadBSPFile( szMapName );

	if( !header )
	{
		Msg( "Couldn't load map \"%s\"\n", szMapName );
		return false;
	}

	memset( BSP::mod_known, 0, sizeof( BSP::mod_known ) );

	m_pModel = &BSP::mod_known[ 0 ];
	BSP::mod_numknown = 1;

	strcpy( m_pModel->name, szMapName );

	bool bSuccess = BSP::LoadBrushModel( m_pModel, header.get() );

	if( bSuccess )
	{
		Msg( "Loaded BSP\n" );

		if( ED_LoadFromFile( m_pModel->entities ) )
		{
			//Set up worldspawn.
			//TODO: how is this supposed to work in the actual engine? - Solokiller
			g_EntList.GetFirstEntity()->KeyValue( "model", m_pModel->name );

			m_Camera.RotateYaw( -90.0f );
		}
		else
		{
			Msg( "Failed to parse entity data\n" );
			bSuccess = false;
			g_EntList.Clear();
		}
	}
	else
	{
		Msg( "Couldn't load BSP\n" );
	}

	return bSuccess;
}

void CMapManager::FreeMap()
{
	if( m_pModel )
	{
		g_EntList.Clear();

		BSP::FreeModel( m_pModel );

		m_pModel = nullptr;
	}
}

void CMapManager::RenderMap( long long uiDeltaTime )
{
	m_flDeltaTime = uiDeltaTime / 1000.0f;

	//Msg( "delta: %f\n", m_flDeltaTime );

	if( m_flDeltaTime > 1.0f )
		m_flDeltaTime = 0.0f;

	//Msg( "delta: %f\n", m_flDeltaTime );

	if( m_flYawVel )
		m_Camera.RotateYaw( m_flDeltaTime * m_flYawVel );

	if( m_flPitchVel )
		m_Camera.RotatePitch( m_flDeltaTime * m_flPitchVel );

	check_gl_error();

	//Depth testing prevents objects that are further away from drawing on top of nearer objects
	glEnable( GL_DEPTH_TEST );

	check_gl_error();

	//Cull back faces
	glEnable( GL_CULL_FACE );

	check_gl_error();

	//We use clockwise order for triangle vertices
	glFrontFace( GL_CW );

	check_gl_error();

	int width, height;

	SDL_GetWindowSize( g_Video.GetWindow(), &width, &height );

	glViewport( 0, 0, width, height );

	const float flAspect = static_cast<float>( width ) / static_cast<float>( height );

	auto projection = glm::perspective( glm::radians( 75.0f ), flAspect, 0.1f, 10000.0f );

	glm::mat4x4 view;

	view = glm::mat4x4(
		0, -1, 0, 0,
		0, 0, 1, 0,
		-1, 0, 0, 0,
		0, 0, 0, 1 );

	view = m_Camera.GetViewMatrix();

	glm::mat4x4 model = glm::mat4x4();

	std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );

	size_t uiCount = 0;

	size_t uiTriangles = 0;

	double flTotal = 0;

	for( CBaseEntity* pEntity = g_EntList.GetFirstEntity(); pEntity; pEntity = g_EntList.GetNextEntity( pEntity ) )
	{
		if( auto pModel = pEntity->GetBrushModel() )
		{
			//TODO: Should tidy up these parameters. - Solokiller
			RenderModel( projection, view, model, pEntity, *pModel, uiCount, uiTriangles, flTotal );
		}
	}

	std::chrono::milliseconds now2 = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );

	//Msg( "Time spent rendering frame (%u polygons, %u triangles, average (msec): %f): %f\n", uiCount, uiTriangles, flTotal / uiCount, ( now2 - now ).count() / 1000.0f );

	//Unbind program
	g_ShaderManager.DeactivateActiveShader();

	check_gl_error();

	//Update screen
	//TODO: handled by VGUI1 - Solokiller
	SDL_GL_SwapWindow( g_Video.GetWindow() );

	check_gl_error();
}

void CMapManager::HandleSDLEvent( SDL_Event& event )
{
	switch( event.type )
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:				KeyEvent( event.key ); break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:		MouseButtonEvent( event.button ); break;

	case SDL_MOUSEMOTION:		MouseMotionEvent( event.motion ); break;

	case SDL_MOUSEWHEEL:		MouseWheelEvent( event.wheel ); break;

	default: break;
	}
}

void CMapManager::RenderModel( const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model, 
							   const CBaseEntity* pEntity, bmodel_t& brushModel, size_t& uiCount, size_t& uiTriangles, double& flTotal )
{
	msurface_t* pSurface = brushModel.surfaces + brushModel.firstmodelsurface;

	CShaderInstance* pShader;

	//TODO: need to sort transparent surfaces - Solokiller
	for( int iIndex = 0; iIndex < brushModel.nummodelsurfaces; ++iIndex, ++pSurface )
	{
		//Sky, origin, aaatrigger, etc. Don't draw these.
		//TODO: add option to draw them.
		if( pSurface->texinfo->flags & TEX_SPECIAL )
			continue;

		pShader = pSurface->texinfo->texture->pShader;

		g_ShaderManager.ActivateShader( pShader, projection, view, model, pEntity );

		glActiveTexture( GL_TEXTURE0 + 1 );

		check_gl_error();

		//Skies will have no texture here.
		glBindTexture( GL_TEXTURE_2D, pSurface->lightmaptexturenum );

		check_gl_error();

		glActiveTexture( GL_TEXTURE0 + 0 );

		check_gl_error();

		for( glpoly_t* pPoly = pSurface->polys; pPoly; pPoly = pPoly->chain )
		{
			if( pSurface->texinfo->texture )
				glBindTexture( GL_TEXTURE_2D, pSurface->texinfo->texture->gl_texturenum );
			else
				glBindTexture( GL_TEXTURE_2D, 0 );

			check_gl_error();

			for( glpoly_t* pPoly2 = pPoly; pPoly2; pPoly2 = pPoly2->next )
			{
				std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );

				glBindBuffer( GL_ARRAY_BUFFER, pPoly2->VBO );

				check_gl_error();

				pShader->SetupVertexAttribs();

				check_gl_error();

				pShader->Draw( pPoly2->numverts );

				++uiCount;

				uiTriangles += pPoly2->numverts - 2;

				std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );

				flTotal += ( end - start ).count();
			}
		}
	}
}

void CMapManager::KeyEvent( const SDL_KeyboardEvent& event )
{
	switch( event.type )
	{
	case SDL_KEYDOWN:
		switch( event.keysym.sym )
		{
		case SDLK_LEFT:		m_flYawVel = -ROTATE_SPEED; break;
		case SDLK_RIGHT:	m_flYawVel = ROTATE_SPEED; break;
		case SDLK_UP:		m_flPitchVel = -ROTATE_SPEED; break;
		case SDLK_DOWN:		m_flPitchVel = ROTATE_SPEED; break;
		default: break;
		}
		break;

	case SDL_KEYUP:
		switch( event.keysym.sym )
		{
		case SDLK_LEFT:		m_flYawVel = 0; break;
		case SDLK_RIGHT:	m_flYawVel = 0; break;
		case SDLK_UP:		m_flPitchVel = 0; break;
		case SDLK_DOWN:		m_flPitchVel = 0; break;
		default: break;
		}
		break;
	}
}

void CMapManager::MouseButtonEvent( const SDL_MouseButtonEvent& event )
{
}

void CMapManager::MouseMotionEvent( const SDL_MouseMotionEvent& event )
{
}

void CMapManager::MouseWheelEvent( const SDL_MouseWheelEvent& event )
{
	const glm::vec3 dir = m_Camera.GetDirection();

	m_Camera.GetPosition() += dir * ( MOVE_SPEED * event.y );
}
