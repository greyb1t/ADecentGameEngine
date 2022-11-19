#include "pch.h"
#include "GeometryShader.h"

Engine::GeometryShader::GeometryShader()
{

}

Engine::GeometryShader::~GeometryShader()
{

}

ID3D11GeometryShader* Engine::GeometryShader::GetGeometryShader()
{
	return myGeometryShader.Get();
}
