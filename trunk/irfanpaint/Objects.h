#pragma once
#ifdef INCLUDE_OBJECTS
#include "Utils.h"
class BWDibSection;
class DibSection;
//Note: all the coords (even those of DCs) are DIB coords relative to the upper-left corner of the DIB unless differently stated
namespace Objects
{
	//Represents an object on the screen
	class Object
	{
	protected:
		//Bounds of the object
		RECT bounds;
	public:
		//Draws the object on the given DC; the ToRepaint parameter can be NULL (in which case all the object must be drawn)
		virtual void Draw(HDC DC, RECT * ToPaint)=0;
		//Returns the bounds of the object
		virtual RECT GetBounds() { return bounds; };
		//Default virtual destructor
		virtual ~Object() { return; };
	};

	class RenderedObject : Object
	{
	protected:
		boost::scoped_ptr<BWDibSection> mask;
		boost::scoped_ptr<DibSection> rendering;
		boost::scoped_ptr<Object> innerObject;
	public:
		//Default constructor
		RenderedObject(Object * InnerObject);
		//Redefined function - acts as proxy
		RECT GetBounds()
		{
			SyncBounds(false);
			return bounds;
		};
		//Sync the bounds of RenderedObject with the ones of innerObject and if necessary eventually updates the rendering
		void SyncBounds(bool UpdateRendering=true);
		//Updates the cached rendering
		void UpdateRendering();
		//Draws it
		void Draw(HDC DC, RECT * ToPaint);
	};

	//Represents a hollow black rectangle; useful for testing purposes
	class HollowRectangle : Object
	{
	public:
		HollowRectangle(RECT Bounds)
		{
			bounds=Bounds;
		};
		void Draw(HDC DC, RECT * ToPaint)
		{
			int dcState=SaveDC(DC);
			SelectObject(DC, GetStockBrush(NULL_BRUSH));
			SelectObject(DC, GetStockPen(BLACK_PEN));
			Rectangle(DC,EXPANDRECT_C(bounds)); 
			RestoreDC(DC,dcState);
		};
	};

	//This object shows the content of a metafile on the screen
	class MetafileObject : Object
	{
	protected:
		//Metafile to be displayed
		HENHMETAFILE metaFile;
	public:
		//Constructs the object; note that the metafile ownership is acquired by the class
		//This overload stretches the metafile to the given bounds
		MetafileObject(RECT Bounds, HENHMETAFILE MetaFile);
		//This overload does not stretch the metafile; it just repositions it
		MetafileObject(POINT InsertPoint, HENHMETAFILE MetaFile);
		//Redefined draw
		void Draw(HDC DC, RECT * ToPaint);
		//Redefined destructor
		~MetafileObject()
		{
			DeleteEnhMetaFile(metaFile);
		};
	};

	//Container of objects
	class ObjectsContainer
	{
	public:
		//Stack of Objects
		typedef std::list<Object *> ObjContainer;
	private:
		//Contains the objects
		ObjContainer objects;
	public:
		//Default constructor
		ObjectsContainer()
		{
			return;
		};
		//Default destructor
		~ObjectsContainer()
		{
			while(objects.size())
			{
				delete objects.front();
				objects.pop_front();
			}
			return;
		};
		//Returns a reference to the objects container
		ObjContainer & GetObjects() { return objects; };

		//Draws the whole objects stack
		void DrawAll(HDC DC, RECT * ToPaint);
	};
}
#endif