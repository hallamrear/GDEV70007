#pragma once

//a plane can be defined as
//f[n|d], such that n represents a normal vector to some point p on the plane
//and w represents the point at which the normal values intersect the plane f
struct Plane
{
	Plane() = default;
	//takes in normal vector of plane
	Plane(const Vector3& n, const float d) :
		x(n.x), y(n.y), z(n.z), w(d)
	{
	};
	//takes in normal components of plane
	Plane(const float nx, const float ny, const float nz, const float d) :
		x(nx), y(ny), z(nz), w(d)
	{
	};

	//returns the normal vector of the plane
	inline const Vector3& Normal()const { return reinterpret_cast<const Vector3&>(x); };
	//sets the plane given a normal and offset from the origin
	void Set(const Vector3& normal, const float& d)
	{
		*reinterpret_cast<Vector3*>(&x) = normal;
		w = d;
	}
	void Set(const float& _x, const float& _y, const float& _z, const float& _d)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _d;
	}
	explicit operator Vector4() const
	{
		return reinterpret_cast<const Vector4&>(*this);
	}

	float& operator[](const int& i) { return ((&x)[i]); };
	const float& operator[](const int& i) const { return ((&x)[i]); };

	//normal vector of plane
	float x;
	float y;
	float z;
	//distance from origin, f*O = O
	float w;//also known as d
};