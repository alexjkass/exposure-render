/*
	Copyright (c) 2011, T. Kroes <t.kroes@tudelft.nl>
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	- Neither the name of the TU Delft nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "General.cuh"

namespace ExposureRender
{

struct Object : public ErObject
{
	Object()
	{
		printf("Object()\n");
	}

	~Object()
	{
		printf("~Object()\n");
	}

	DEVICE_NI void Intersect(const Ray& R, ScatterEvent& RS)
	{
		Ray Rt = TransformRay(Shape.InvTM, R);

		Intersection Int;

		IntersectShape(Shape, Rt, Int);

		if (Int.Valid)
		{
			RS.Valid	= true;
			RS.N 		= TransformVector(Shape.TM, Int.N);
			RS.P 		= TransformPoint(Shape.TM, Int.P);
			RS.T 		= Length(RS.P - R.O);
			RS.Wo		= -R.D;
			RS.Le		= ColorXYZf(0.0f);
			RS.UV		= Int.UV;
		}
	}

	DEVICE_NI bool Intersects(const Ray& R)
	{
		return IntersectsShape(Shape, TransformRay(Shape.InvTM, R));
	}

	HOST Object& Object::operator = (const ErObject& Other)
	{
		this->Enabled				= Other.Enabled;
		this->Shape					= Other.Shape;
		this->DiffuseTextureID		= Other.DiffuseTextureID;
		this->SpecularTextureID		= Other.SpecularTextureID;
		this->GlossinessTextureID	= Other.GlossinessTextureID;
		this->Ior					= Other.Ior;

		return *this;
	}
};

typedef ResourceList<Object, MAX_NO_OBJECTS> Objects;

DEVICE Objects& GetObjects()
{
	return *((Objects*)gpObjects);
}

DEVICE_NI void IntersectObjects(const Ray& R, ScatterEvent& RS)
{
	float T = FLT_MAX;

	for (int i = 0; i < GetObjects().Count; i++)
	{
		Object& Object = GetObjects().Get(i);

		ScatterEvent LocalRS(ScatterEvent::Object);

		LocalRS.ObjectID = i;

		Object.Intersect(R, LocalRS);

		if (LocalRS.Valid && LocalRS.T < T)
		{
			RS = LocalRS;
			T = LocalRS.T;
		}
	}
}

DEVICE_NI bool IntersectsObject(const Ray& R)
{
	for (int i = 0; i < GetObjects().Count; i++)
	{
		if (GetObjects().Get(i).Intersects(R))
			return true;
	}

	return false;
}

}