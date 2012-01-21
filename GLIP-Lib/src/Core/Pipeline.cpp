/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : GPLv3                                                                                     */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : Pipeline.cpp                                                                              */
/*     Original Date : August 15th 2011                                                                          */
/*                                                                                                               */
/*     Description   : Pipeline object                                                                           */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    Pipeline.cpp
 * \brief   Pipeline object
 * \author  R. KERVICHE
 * \date    August 15th 2011
**/

#include <limits>
#include "Pipeline.hpp"
#include "Component.hpp"
#include "HdlFBO.hpp"
#include "devDebugTools.hpp"

	using namespace Glip::CoreGL;
	using namespace Glip::CorePipeline;

// __ReadOnly_PipelineLayout
	/**
	\fn __ReadOnly_PipelineLayout::__ReadOnly_PipelineLayout(const std::string& type)
	\brief __ReadOnly_PipelineLayout constructor.
	\param type Typename of the pipeline.
	**/
	__ReadOnly_PipelineLayout::__ReadOnly_PipelineLayout(const std::string& type)
	 : __ReadOnly_ComponentLayout(type)
	{ }

	/**
	\fn __ReadOnly_PipelineLayout::__ReadOnly_PipelineLayout(const __ReadOnly_PipelineLayout& c)
	\brief __ReadOnly_PipelineLayout constructor.
	\param c Copy.
	**/
	__ReadOnly_PipelineLayout::__ReadOnly_PipelineLayout(const __ReadOnly_PipelineLayout& c)
	 : __ReadOnly_ComponentLayout(c)
	{
		//std::cout << "Starting copy of pipeline layout for " << getNameExtended() << std::endl;
		// Copy of the whole vector
		elementsKind   	= c.elementsKind;
		elementsID     	= c.elementsID;
		connections	= c.connections;

		for(int i=0; i<c.elementsLayout.size(); i++)
		{
			switch(elementsKind[i])
			{
				case FILTER:
					elementsLayout.push_back(reinterpret_cast<__ReadOnly_ComponentLayout*>(new __ReadOnly_FilterLayout(c.filterLayout(i))));
					break;
				case PIPELINE:
					elementsLayout.push_back(reinterpret_cast<__ReadOnly_ComponentLayout*>(new __ReadOnly_PipelineLayout(c.pipelineLayout(i))));
					break;
				default:
				throw Exception("__ReadOnly_PipelineLayout::__ReadOnly_PipelineLayout - Unknown type for copy", __FILE__, __LINE__);
			}
		}
		//std::cout << "end copy of pipeline layout for " << getNameExtended() << std::endl;
	}

	/**
	\fn int __ReadOnly_PipelineLayout::getElementID(int i)
	\brief Get element ID in global structure.
	\param i The ID of the element in the local pipeline layout.
	\return The ID of the element in the global structure or raise an exception if any errors occur.
	**/
	int __ReadOnly_PipelineLayout::getElementID(int i)
	{
		checkElement(i);
		return elementsID[i];
	}

	/**
	\fn void __ReadOnly_PipelineLayout::setElementID(int i, int ID)
	\brief Set element ID in global structure or raise an exception if any errors occur.
	\param i The ID of the element in the local pipeline layout.
	\param ID The ID of the element in the global structure.
	**/
	void __ReadOnly_PipelineLayout::setElementID(int i, int ID)
	{
		checkElement(i);
		elementsID[i] = ID;
	}

	/**
	\fn __ReadOnly_PipelineLayout::Connection __ReadOnly_PipelineLayout::getConnection(int i) const
	\brief Get the connection by its ID.
	\param i The ID of the connection.
	\return A copy of the corresponding Connection object or raise an exception if any errors occur.
	**/
	__ReadOnly_PipelineLayout::Connection __ReadOnly_PipelineLayout::getConnection(int i) const
	{
		if(i<0 || i>=connections.size())
			throw Exception("__ReadOnly_PipelineLayout::getConnection - Bad connection ID for "  + getNameExtended() + " ID : " + to_string(i), __FILE__, __LINE__);
		return connections[i];
	}

	/**
	\fn void __ReadOnly_PipelineLayout::checkElement(int i) const
	\brief Check if element exists and raise an exception if any errors occur.
	\param i The ID of the element.
	**/
	void __ReadOnly_PipelineLayout::checkElement(int i) const
	{
		if(i<0 || i>=elementsLayout.size())
			throw Exception("__ReadOnly_PipelineLayout::checkElement - Bad element ID for "  + getNameExtended() + " ID : " + to_string(i), __FILE__, __LINE__);
	}

	/**
	\fn int __ReadOnly_PipelineLayout::getNumElements(void) const
	\brief Get the number of elements.
	\return Number of elements.
	**/
	int __ReadOnly_PipelineLayout::getNumElements(void) const
	{
		return elementsLayout.size();
	}

	/**
	\fn int __ReadOnly_PipelineLayout::getNumConnections(void) const
	\brief Get the number of connections.
	\return Number of connections.
	**/
	int __ReadOnly_PipelineLayout::getNumConnections(void) const
	{
		return connections.size();
	}

	/**
	\fn void __ReadOnly_PipelineLayout::getInfoElements(int& numFilters, int& numPipelines)
	\brief Get the total number of Filters and Pipelines contained by this pipeline.
	\param numFilters The total number of filters.
	\param numPipelines The total number of pipelines (including this one).
	**/
	void __ReadOnly_PipelineLayout::getInfoElements(int& numFilters, int& numPipelines)
	{
		int a, b;
		__ReadOnly_PipelineLayout* tmp = NULL;
		numFilters  	= 0;
		numPipelines	= 0;

		for(int i=0; i<elementsLayout.size(); i++)
		{
			switch(elementsKind[i])
			{
				case FILTER:
					numFilters++;
					break;
				case PIPELINE:
					tmp = reinterpret_cast<__ReadOnly_PipelineLayout*>(elementsLayout[i]);
					tmp->getInfoElements(a,b);
					numFilters   += a;
					numPipelines += b+1;
					break;
				default:
					throw Exception("__ReadOnly_PipelineLayout::getInfoElements - Unknown type", __FILE__, __LINE__);
			}
		}

		numPipelines++; // include this
	}

	/**
	\fn int __ReadOnly_PipelineLayout::getElementIndex(const std::string& name) const
	\brief Get the ID of an element knowing its name.
	\param name The name of the element.
	\return The ID of the element or raise an exception if any errors occur.
	**/
	int __ReadOnly_PipelineLayout::getElementIndex(const std::string& name) const
	{
		int index;

		try
		{
			index = getIndexByNameFct(name, elementsLayout.size(), __ReadOnly_PipelineLayout::componentName, reinterpret_cast<const void*>(this));
		}
		catch(std::exception& e)
		{
			Exception m("getElementIndex - Caught an exception while looking for " + name + " in " + getNameExtended(), __FILE__, __LINE__);
			throw m+e;
		}

		return index;
	}

	/**
	\fn bool __ReadOnly_PipelineLayout::doesElementExist(const std::string& name) const
	\brief Check if an element exists knowing its name.
	\param name The name of the element.
	\return True if such an element exists, False otherwise.
	**/
	bool __ReadOnly_PipelineLayout::doesElementExist(const std::string& name) const
	{
		return doesElementExistByNameFct(name, elementsLayout.size(), __ReadOnly_PipelineLayout::componentName, reinterpret_cast<const void*>(this));
	}

	/**
	\fn ComponentKind __ReadOnly_PipelineLayout::getElementKind(int i) const
	\brief Get the kind of an element.
	\param i The ID of the element.
	\return The kind of the element or raise an exception if any errors occur.
	**/
	ComponentKind __ReadOnly_PipelineLayout::getElementKind(int i) const
	{
		checkElement(i);
		return elementsKind[i];
	}

	/**
	\fn ObjectName& __ReadOnly_PipelineLayout::componentName(int i, const void* obj)
	\brief Get the name of an element (selection function).
	\param i The ID of the element.
	\param obj The source object.
	\return The name of the element or raise an exception if any errors occur.
	**/
	ObjectName& __ReadOnly_PipelineLayout::componentName(int i, const void* obj)
	{
		return reinterpret_cast<const __ReadOnly_PipelineLayout*>(obj)->componentLayout(i);
	}

	/**
	\fn __ReadOnly_ComponentLayout& __ReadOnly_PipelineLayout::componentLayout(int i) const
	\brief Get the component layout by its index.
	\param i The ID of the component.
	\return A reference to the component or raise an exception if any errors occur.
	**/
	__ReadOnly_ComponentLayout& __ReadOnly_PipelineLayout::componentLayout(int i) const
	{
		checkElement(i);

		//std::cout << "ACCESSING COMPONENT (int)" << std::endl;
		//return *elementsLayout[i];
		switch(elementsKind[i])
		{
			case FILTER:
				return *reinterpret_cast<__ReadOnly_FilterLayout*>(elementsLayout[i]);
			case PIPELINE:
				return *reinterpret_cast<__ReadOnly_PipelineLayout*>(elementsLayout[i]);
			default :
				throw Exception("__ReadOnly_PipelineLayout::componentLayout - Type not recognized",__FILE__, __LINE__);
		}
	}

	/**
	\fn __ReadOnly_ComponentLayout& __ReadOnly_PipelineLayout::componentLayout(const std::string& name) const
	\brief Get the component layout by its name.
	\param name The name of the element.
	\return A reference to the component or raise an exception if any errors occur.
	**/
	__ReadOnly_ComponentLayout& __ReadOnly_PipelineLayout::componentLayout(const std::string& name) const
	{
		int index = getElementIndex(name);
		//std::cout << "ACCESSING COMPONENT (int)" << std::endl;
		return componentLayout(index);
	}

	/**
	\fn __ReadOnly_FilterLayout& __ReadOnly_PipelineLayout::filterLayout(int i) const
	\brief Get the filter layout by its index.
	\param i The ID of the filter layout.
	\return A reference to the filter layout or raise an exception if any errors occur.
	**/
	__ReadOnly_FilterLayout& __ReadOnly_PipelineLayout::filterLayout(int i) const
	{
		//std::cout << "ACCESSING FILTER (int)" << std::endl;
		checkElement(i);
		if(getElementKind(i)!=FILTER)
			throw Exception("__ReadOnly_PipelineLayout::filterLayout - This element exists but is not a filter!");
		return *reinterpret_cast<__ReadOnly_FilterLayout*>(elementsLayout[i]);
	}

	/**
	\fn __ReadOnly_FilterLayout& __ReadOnly_PipelineLayout::filterLayout(const std::string& name) const
	\brief Get the filter layout by its name.
	\param name The name of the filter layout.
	\return A reference to the filter layout or raise an exception if any errors occur.
	**/
	__ReadOnly_FilterLayout& __ReadOnly_PipelineLayout::filterLayout(const std::string& name) const
	{
		//std::cout << "ACCESSING FILTER (name)" << std::endl;
		int index = getElementIndex(name);
		if(getElementKind(index)!=FILTER)
			throw Exception("__ReadOnly_PipelineLayout::filterLayout - This element exists but is not a filter!");
		return *reinterpret_cast<__ReadOnly_FilterLayout*>(elementsLayout[index]);
	}

	/**
	\fn __ReadOnly_PipelineLayout& __ReadOnly_PipelineLayout::pipelineLayout(int i) const
	\brief Get the pipeline layout by its index.
	\param i The ID of the pipeline layout.
	\return A reference to the pipeline layout or raise an exception if any errors occur.
	**/
	__ReadOnly_PipelineLayout& __ReadOnly_PipelineLayout::pipelineLayout(int i) const
	{
		//std::cout << "ACCESSING PIPELINE (int)" << std::endl;
		checkElement(i);
		if(getElementKind(i)!=PIPELINE)
			throw Exception("__ReadOnly_PipelineLayout::pipelineLayout - This element exists but is not a pipeline!");
		return *reinterpret_cast<__ReadOnly_PipelineLayout*>(elementsLayout[i]);
	}

	/**
	\fn __ReadOnly_PipelineLayout& __ReadOnly_PipelineLayout::pipelineLayout(const std::string& name) const
	\brief Get the pipeline layout by its name.
	\param name The name of the pipeline layout.
	\return A reference to the pipeline layout or raise an exception if any errors occur.
	**/
	__ReadOnly_PipelineLayout& __ReadOnly_PipelineLayout::pipelineLayout(const std::string& name) const
	{
		//std::cout << "ACCESSING PIPELINE (name)" << std::endl;
		int index = getElementIndex(name);
		if(getElementKind(index)!=PIPELINE)
			throw Exception("__ReadOnly_PipelineLayout::pipelineLayout - This element exists but is not a pipeline!");
		return *reinterpret_cast<__ReadOnly_PipelineLayout*>(elementsLayout[index]);
	}

	/**
	\fn std::vector<__ReadOnly_PipelineLayout::Connection> __ReadOnly_PipelineLayout::getConnectionDestinations(int id, int p)
	\brief Get all destinations of an output port.
	\param id The ID of the output element.
	\param p The port of the output element.
	\return A vector of Conection object, all having output as (id,p) or raise an exception if any errors occur.
	**/
	std::vector<__ReadOnly_PipelineLayout::Connection> __ReadOnly_PipelineLayout::getConnectionDestinations(int id, int p)
	{
		if(id!=THIS_PIPELINE)
		{
			__ReadOnly_ComponentLayout& src = componentLayout(id);
			src.checkOutputPort(p);
		}
		else
			checkInputPort(p);

		// The Element and its port exist, now find their connexions
		std::vector<Connection> result;
		for(std::vector<Connection>::iterator it=connections.begin(); it!=connections.end(); it++)
			if( (*it).idOut==id && (*it).portOut==p) result.push_back(*it);

		return result;
	}

	/**
	\fn __ReadOnly_PipelineLayout::Connection __ReadOnly_PipelineLayout::getConnectionSource(int id, int p)
	\brief Get the source of an input port.
	\param id The ID of the input element.
	\param p The port of the input element.
	\return A Conection object, having input as (id,p) or raise an exception if any errors occur.
	**/
	__ReadOnly_PipelineLayout::Connection __ReadOnly_PipelineLayout::getConnectionSource(int id, int p)
	{
		std::string str;

		if(id!=THIS_PIPELINE)
		{
			__ReadOnly_ComponentLayout& src = componentLayout(id);
			src.checkInputPort(p);
		}
		else
			checkOutputPort(p);

		// The Element and its port exist, now find the connexion
		for(std::vector<Connection>::iterator it=connections.begin(); it!=connections.end(); it++)
			if( (*it).idIn==id && (*it).portIn==p) return (*it);

		if(id!=THIS_PIPELINE)
		{
			__ReadOnly_ComponentLayout& src = componentLayout(id);
			throw Exception("Element " + src.getNameExtended() + " has no source on output port " + src.getInputPortNameExtended(p), __FILE__, __LINE__);
		}
		else
			throw Exception("This Pipeline " + getNameExtended() + " has no source on output port " + getOutputPortNameExtended(p), __FILE__, __LINE__);
	}

	/**
	\fn std::string __ReadOnly_PipelineLayout::getConnectionDestinationsName(int source, int port)
	\brief Get the name of the elements linked to the output.
	\param source The ID of the output element.
	\param port The port of the output element.
	\return A standard string containing all the elements name linked to the output or raise an exception if any errors occur.
	**/
	std::string __ReadOnly_PipelineLayout::getConnectionDestinationsName(int source, int port)
	{
		std::vector<Connection> res = getConnectionDestinations(source, port);
		std::string result;

		for(std::vector<Connection>::iterator it=res.begin(); it!=res.end(); it++)
		{
			__ReadOnly_ComponentLayout& tmp = componentLayout((*it).idIn);
			result += tmp.getNameExtended() + SEPARATOR + tmp.getInputPortNameExtended((*it).portIn) + "\n";
		}

		return result;
	}

	/**
	\fn std::string __ReadOnly_PipelineLayout::getConnectionDestinationsName(const std::string& source, const std::string& port)
	\brief Get the name of the elements linked to the output.
	\param source The name of the output element.
	\param port The name of the port of the output element.
	\return A standard string containing all the elements name linked to the output or raise an exception if any errors occur.
	**/
	std::string __ReadOnly_PipelineLayout::getConnectionDestinationsName(const std::string& source, const std::string& port)
	{
		int id = getElementIndex(source);
		int p  = componentLayout(id).getOutputPortID(port);
		return getConnectionDestinationsName(id, p);
	}

	/**
	\fn std::string __ReadOnly_PipelineLayout::getConnectionSourceName(int dest, int port)
	\brief Get the name of the element linked to the input.
	\param dest The ID of the input element.
	\param port The port of the input element.
	\return A standard string containing the element name linked to the input or raise an exception if any errors occur.
	**/
	std::string __ReadOnly_PipelineLayout::getConnectionSourceName(int dest, int port)
	{
		Connection c = getConnectionSource(dest, port);
		__ReadOnly_ComponentLayout& tmp = componentLayout(c.idOut);
		return tmp.getNameExtended() + SEPARATOR + tmp.getInputPortNameExtended(c.portOut) + "\n";
	}

	/**
	\fn std::string __ReadOnly_PipelineLayout::getConnectionSourceName(const std::string& dest, const std::string& port)
	\brief Get the name of the element linked to the input.
	\param dest The name of the input element.
	\param port The name of the port of the input element.
	\return A standard string containing the element name linked to the input or raise an exception if any errors occur.
	**/
	std::string __ReadOnly_PipelineLayout::getConnectionSourceName(const std::string& dest, const std::string& port)
	{
		int id = getElementIndex(dest);
		int p  = componentLayout(id).getOutputPortID(port);
		return getConnectionSourceName(id, p);
	}

	/**
	\fn bool __ReadOnly_PipelineLayout::check(bool exception)
	\brief Check the validity of the pipeline layout.
	\param exception If set to true, an exception would be raised if any error is found.
	\return true if valid, false otherwise.
	**/
	bool __ReadOnly_PipelineLayout::check(bool exception)
	{
		std::string res;

		for(int i=0; i<elementsLayout.size(); i++)
		{
			__ReadOnly_ComponentLayout& tmp = componentLayout(i);
			for(int j=0; j<tmp.getNumInputPort(); j++)
			{
				try
				{
					getConnectionSource(i, j);
				}
				catch(std::exception& e)
				{
					res += e.what();
					res += '\n';
				}
			}
		}

		for(int i=0; i<getNumOutputPort(); i++)
		{
			try
			{
				getConnectionSource(THIS_PIPELINE, i);
			}
			catch(std::exception& e)
			{
				res += e.what();
				res += '\n';
			}
		}

		if(exception && !res.empty())
			throw Exception("check - The following errors has been found in the PipelineLayout " + getNameExtended() + " : \n" + res, __FILE__, __LINE__);
		else
			return false;

		return true;
	}

// PipelineLayout
	/**
	\fn PipelineLayout::PipelineLayout(const std::string& type)
	\brief PipelineLayout constructor.
	\param type The typename.
	**/
	PipelineLayout::PipelineLayout(const std::string& type)
	 : __ReadOnly_ComponentLayout(type), ComponentLayout(type), __ReadOnly_PipelineLayout(type)
	{ }

	/**
	\fn PipelineLayout::PipelineLayout(__ReadOnly_PipelineLayout& c)
	\brief PipelineLayout constructor.
	\param c Copy.
	**/
	PipelineLayout::PipelineLayout(__ReadOnly_PipelineLayout& c)
	 : __ReadOnly_PipelineLayout(c), __ReadOnly_ComponentLayout(c), ComponentLayout(c)
	{ }

	/**
	\fn int PipelineLayout::add(const __ReadOnly_FilterLayout& filterLayout, const std::string& name)
	\brief Ad a filter to the pipeline layout.
	\param filterLayout The filter layout.
	\param name The name of the element.
	\return The ID of the element added.
	**/
	int PipelineLayout::add(const __ReadOnly_FilterLayout& filterLayout, const std::string& name)
	{
		if(doesElementExist(name))
			throw Exception("PipelineLayout::add - An element with the name " + name + " already exists.", __FILE__, __LINE__);

		__ReadOnly_FilterLayout* tmp = new __ReadOnly_FilterLayout(filterLayout);
		tmp->setName(name);
		std::cout << "Test : <" << tmp->getName() << '>' << std::endl;
		elementsLayout.push_back(reinterpret_cast<__ReadOnly_ComponentLayout*>(tmp));
		elementsKind.push_back(FILTER);
		elementsID.push_back(ELEMENT_NOT_ASSOCIATED);
		return elementsLayout.size()-1;
	}

	/**
	\fn int PipelineLayout::add(const __ReadOnly_PipelineLayout& pipelineLayout, const std::string& name)
	\brief Ad a subpipeline to the pipeline layout.
	\param pipelineLayout The pipeline layout.
	\param name The name of the element.
	\return The ID of the element added.
	**/
	int PipelineLayout::add(const __ReadOnly_PipelineLayout& pipelineLayout, const std::string& name)
	{
		if(doesElementExist(name))
			throw Exception("PipelineLayout::add - An element with the name " + name + " already exists.", __FILE__, __LINE__);

		__ReadOnly_PipelineLayout* tmp = new __ReadOnly_PipelineLayout(pipelineLayout);
		tmp->setName(name);
		elementsLayout.push_back(reinterpret_cast<__ReadOnly_ComponentLayout*>(tmp));
		elementsKind.push_back(PIPELINE);
		elementsID.push_back(ELEMENT_NOT_ASSOCIATED);
		return elementsLayout.size()-1;
	}

	/**
	\fn int PipelineLayout::addInput(const std::string& name)
	\brief Add an input port to the pipeline layout.
	\param name The name of the new input port.
	\return The ID of the new input port.
	**/
	int PipelineLayout::addInput(const std::string& name)
	{
		return addInputPort(name);
	}

	/**
	\fn int PipelineLayout::addOutput(const std::string& name)
	\brief Add an output port to the pipeline layout.
	\param name The name of the new output port.
	\return The ID of the new output port.
	**/
	int PipelineLayout::addOutput(const std::string& name)
	{
		return addOutputPort(name);
	}

	/**
	\fn void PipelineLayout::connect(int filterOut, int portOut, int filterIn, int portIn)
	\brief Create a connection between two elements or an element and this pipeline and raise an exception if any errors occur.
	\param filterOut The ID of the output element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portOut The ID of the output port.
	\param filterIn The ID of the input element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portIn The ID of the input port.
	**/
	void PipelineLayout::connect(int filterOut, int portOut, int filterIn, int portIn)
	{
		if(filterOut==THIS_PIPELINE && filterIn==THIS_PIPELINE)
			throw Exception("PipelineLayout::connect - can't connect directly an input to an output, you don't need that!", __FILE__, __LINE__);

		if(filterOut!=THIS_PIPELINE)
		{
			__ReadOnly_ComponentLayout& fo = componentLayout(filterOut); // Source
			fo.checkOutputPort(portOut);
		}
		else
			checkInputPort(portOut);

		if(filterIn!=THIS_PIPELINE)
		{
			__ReadOnly_ComponentLayout& fi = componentLayout(filterIn);  // Destination
			fi.checkInputPort(portIn);
		}
		else
			checkOutputPort(portIn);

		// Check if a connexion already exist to the destination :
		for(std::vector<Connection>::iterator it=connections.begin(); it!=connections.end(); it++)
			if( (*it).idIn==filterIn && (*it).portIn==portIn)
				if(filterIn!=THIS_PIPELINE)
					throw Exception("PipelineLayout::connect - A connexion already exists to the destination : " + componentLayout(filterIn).getNameExtended() + " on port " + componentLayout(filterIn).getInputPortNameExtended(portIn), __FILE__, __LINE__);
				else
					throw Exception("PipelineLayout::connect - A connexion already exists to this pipeline output : " + getNameExtended() + " on port " + getInputPortNameExtended(portIn), __FILE__, __LINE__);

		Connection c;
		c.idOut   = filterOut;
		c.portOut = portOut;
		c.idIn    = filterIn;
		c.portIn  = portIn;

		//std::cout << "Connexion de " << filterOut << ':' << portOut << " à " << filterIn << ':' << portIn << std::endl;

		connections.push_back(c);
	}

	/**
	\fn void PipelineLayout::connect(const std::string& filterOut, const std::string& portOut, const std::string& filterIn, const std::string& portIn)
	\brief Create a connection between two elements or an element and this pipeline and raise an exception if any errors occur.
	\param filterOut The name of the output element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portOut The name of the output port.
	\param filterIn The name of the input element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portIn The name of the input port.
	**/
	void PipelineLayout::connect(const std::string& filterOut, const std::string& portOut, const std::string& filterIn, const std::string& portIn)
	{
		int fi = getElementIndex(filterIn),
		fo = getElementIndex(filterOut),
		pi = componentLayout(filterIn).getInputPortID(portIn),
		po = componentLayout(filterOut).getOutputPortID(portOut);

		connect(fo, po, fi, pi); // Check-in done twice but...
	}

	/**
	\fn void PipelineLayout::connectToInput(int port, int filterIn,  int portIn)
	\brief Create a connection between an input port of this pipeline and one of its element and raise an exception if any errors occur.
	\param port The ID of the port for this pipeline.
	\param filterIn The ID of the input element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portIn The ID of the input port.
	**/
	void PipelineLayout::connectToInput(int port, int filterIn,  int portIn)
	{
		connect(THIS_PIPELINE, port, filterIn, portIn);
	}

	/**
	void PipelineLayout::connectToInput(const std::string& port, const std::string& filterIn, const std::string& portIn)
	\brief Create a connection between an input port of this pipeline and one of its element and raise an exception if any errors occur.
	\param port The name of the port for this pipeline.
	\param filterIn The name of the input element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portIn The name of the input port.
	**/
	void PipelineLayout::connectToInput(const std::string& port, const std::string& filterIn, const std::string& portIn)
	{
		try
		{
			int p  = getInputPortID(port),
			fi = getElementIndex(filterIn),
			pi = componentLayout(filterIn).getInputPortID(portIn);
			connect(THIS_PIPELINE, p, fi, pi);
		}
		catch(std::exception& e)
		{
			Exception m("PipelineLayout::connectToInput (str) - Caught an exception for the object " + getNameExtended(), __FILE__, __LINE__);
			throw m+e;
		}
	}

	/**
	\fn void PipelineLayout::connectToOutput(int filterOut, int portOut, int port)
	\brief Create a connection between an output port of this pipeline and one of its element and raise an exception if any errors occur.
	\param filterOut The ID of the output element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portOut The ID of the output port.
	\param port The ID of the port for this pipeline.
	**/
	void PipelineLayout::connectToOutput(int filterOut, int portOut, int port)
	{
		connect(filterOut, portOut, THIS_PIPELINE, port);
	}

	/**
	\fn void PipelineLayout::connectToOutput(const std::string& filterOut, const std::string& portOut, const std::string& port)
	\brief Create a connection between an output port of this pipeline and one of its element and raise an exception if any errors occur.
	\param filterOut The name of the output element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portOut The name of the output port.
	\param port The name of the port for this pipeline.
	**/
	void PipelineLayout::connectToOutput(const std::string& filterOut, const std::string& portOut, const std::string& port)
	{
		try
		{
			int p  = getOutputPortID(port),
			fo = getElementIndex(filterOut),
			po = componentLayout(filterOut).getOutputPortID(portOut);
			connect(fo, po, THIS_PIPELINE, p);
		}
		catch(std::exception& e)
		{
			Exception m("PipelineLayout::connectToOutput (str) - Caught an exception for the object " + getNameExtended(), __FILE__, __LINE__);
			throw m+e;
		}
	}

// Pipeline
	/**
	\fn Pipeline::Pipeline(__ReadOnly_PipelineLayout& p, const std::string& name)
	\brief Pipeline constructor.
	\param p Pipeline layout.
	\param name Name of the pipeline.
	**/
	Pipeline::Pipeline(__ReadOnly_PipelineLayout& p, const std::string& name)
	 : __ReadOnly_ComponentLayout(p), __ReadOnly_PipelineLayout(p), Component(p, name), perfsMonitoring(false)
	{
		cleanInput();
		outputBuffer.assign(getNumOutputPort(), 0);
                outputBufferPort.assign(getNumOutputPort(), 0);

		build();
	}

	Pipeline::~Pipeline(void)
	{
		cleanInput();

		for(std::vector<TableIndex*>::iterator it = listOfArgBuffersOutput.begin(); it!=listOfArgBuffersOutput.end(); it++)
			delete (*it);
		listOfArgBuffersOutput.clear();

		for(std::vector<TableIndex*>::iterator it = listOfArgBuffers.begin(); it!=listOfArgBuffers.end(); it++)
			delete (*it);
		listOfArgBuffers.clear();

		actionFilter.clear();
		outputBuffer.clear();
		outputBufferPort.clear();

		for(TableBuffer::iterator it = buffers.begin(); it!=buffers.end(); it++)
			delete (*it);
		buffers.clear();

		for(TableFilter::iterator it = filters.begin(); it!=filters.end(); it++)
			delete (*it);
		filters.clear();
	}

	/**
	\fn void Pipeline::cleanInput(void)
	\brief Clean all inputs from previously acquired texture pointers.
	**/
	void Pipeline::cleanInput(void)
	{
		input.clear();
	}

	/**
	\fn void Pipeline::build(void)
	\brief Build pipeline process from layout information, choosing the best path.
	**/
	void Pipeline::build(void)
	{
		int dummy;

		#ifdef __DEVELOPMENT_VERBOSE__
			std::cout << "Building pipeline " << getNameExtended() << std::endl;
		#endif

		try
		{
			// 1st Step

			// Push this as the first item of the wait list
			// While the wait list is not empty
			//     	Take the first/last element in the list and look for all its elements
			//          	If the element is a pipeline, push it in the wait list and add it an INDEX
			//          	If the element is a filter, create an instance of it and set its ID
			//     	Done.
			//	For each link of the current pipeline :
			//		Change coordinates to absolute ones
			//	Done.
			// Done.

			#ifdef __DEVELOPMENT_VERBOSE__
				std::cout << "First step" << std::endl;
			#endif

			int startPipeline = 0;
			std::vector<__ReadOnly_PipelineLayout*> pipeList;
			std::list<int> waitList;
			TableConnection  tmpConnections;
			std::vector<int> connectionOwner;

			getInfoElements(startPipeline, dummy); //startPipeline will contain the number of filter
			pipeList.push_back(this);
			waitList.push_back(startPipeline);

			while(!waitList.empty())
			{
				int 	currentPipeline	= waitList.front(),
				 	offsetPipeline	= pipeList.size()+startPipeline,
					offsetFilter	= filters.size();
				__ReadOnly_PipelineLayout* tmp = pipeList[currentPipeline-startPipeline];

				// Create instance of all elements :
				for(int i=0; i<tmp->getNumElements(); i++)
				{
					switch(tmp->getElementKind(i))
					{
					case FILTER :
						#ifdef __DEVELOPMENT_VERBOSE__
							std::cout << "    Adding a new filter" << std::endl;
						#endif
						filters.push_back(new Filter(tmp->filterLayout(i)));
						tmp->setElementID(i, filters.size()-1);
						#ifdef __DEVELOPMENT_VERBOSE__
							std::cout << "    Adding : " << filters.back()->getNameExtended() << std::endl;
							std::cout << "    ID     : " << filters.size()-1 << std::endl;
						#endif
						break;
					case PIPELINE :
						#ifdef __DEVELOPMENT_VERBOSE__
							std::cout << "    Adding a new Pipeline" << std::endl;
						#endif
						pipeList.push_back(&tmp->pipelineLayout(i));
						waitList.push_back(pipeList.size()-1 + startPipeline);
						tmp->setElementID(i, pipeList.size()-1 + startPipeline);
						#ifdef __DEVELOPMENT_VERBOSE__
							std::cout << "    Adding : " << pipeList.back()->getNameExtended() << std::endl;
							std::cout << "    ID     : " << pipeList.size()-1 + startPipeline << std::endl;
						#endif
						break;
					default :
						throw Exception("Pipeline::build - Element type not recognized for " + tmp->componentLayout(i).getNameExtended(), __FILE__, __LINE__);
					}
				}

				// Save all the connections to absolute basis :
				#ifdef __DEVELOPMENT_VERBOSE__
					std::cout << "Adding " << tmp->getNumConnections() << " connections from " << tmp->getNameExtended() << " (Current pipeline ID : " << currentPipeline << ", offset = " << offsetPipeline << ", offsetFilter = " << offsetFilter << ')' << std::endl;
				#endif
				for(int i=0; i<tmp->getNumConnections(); i++)
				{
					Connection c = tmp->getConnection(i);

					#ifdef __DEVELOPMENT_VERBOSE__
						std::cout << "    Connection BEFORE  : " << std::endl;
						std::cout << "        idIn    : " << c.idIn << std::endl;
						std::cout << "        portIn  : " << c.portIn << std::endl;
						std::cout << "        idOut   : " << c.idOut << std::endl;
						std::cout << "        portOut : " << c.portOut << std::endl;
					#endif

					// Replace to absolute coordinates :
					if(c.idIn==THIS_PIPELINE)
					{
						if(currentPipeline>startPipeline) // Not main pipeline
							c.idIn = currentPipeline;
						// else nothing, THIS_PIPELINE is valid
					}
					else
					{
						if(tmp->getElementKind(c.idIn)==PIPELINE)
							//OLD : c.idIn = c.idIn + offsetPipeline;
							c.idIn = tmp->getElementID(c.idIn);
						else
							//OLD : c.idIn = c.idIn + offsetFilter;
							c.idIn = tmp->getElementID(c.idIn);
					}
					if(c.idOut==THIS_PIPELINE)
					{
						if(currentPipeline>startPipeline)
							c.idOut = currentPipeline;
						//else nothing, THIS_PIPELINE is valid
					}
					else
					{
						if(tmp->getElementKind(c.idOut)==PIPELINE)
							//OLD : c.idOut = c.idOut + offsetPipeline;
							c.idOut = tmp->getElementID(c.idOut);
						else
							//OLD : c.idOut = c.idOut + offsetFilter;
							c.idOut = tmp->getElementID(c.idOut);
					}

					#ifdef __DEVELOPMENT_VERBOSE__
						std::cout << "    Connection AFTER : " << std::endl;
						std::cout << "        idIn    : " << c.idIn << std::endl;
						std::cout << "        portIn  : " << c.portIn << std::endl;
						std::cout << "        idOut   : " << c.idOut << std::endl;
						std::cout << "        portOut : " << c.portOut << std::endl;
					#endif

					tmpConnections.push_back(c);
					connectionOwner.push_back(currentPipeline);
				}

				waitList.pop_front();
			}

			// 2nd Step

			// Do
			//	For each links in tmpConnections
			//		If the idOut is a Pipeline and not THIS_PIPELINE
			//			For each links in tmpConnections
			//				If current links input is the same as upper links output
			//					Add a link using input of upperlinks and output of  this link
			//					Remove this link
			//			Done.
			//	Done.
			//	Update proposition (there is Pipeline links in tmpConnections)
			// While (there is Pipeline links in tmpConnections)

			#ifdef __DEVELOPMENT_VERBOSE__
				std::cout << "Second step (Num connections : " << tmpConnections.size() << ')' << std::endl;
			#endif
			bool test = true;

			#ifdef __VERBOSE__
				std::cout << "Connection list : " << tmpConnections.size() << std::endl;
				for(TableConnection::iterator it=tmpConnections.begin(); it!=tmpConnections.end(); it++)
				{
					std::cout << "    Connection  : " << std::endl;
					std::cout << "        idIn    : " << (*it).idIn << std::endl;
					std::cout << "        portIn  : " << (*it).portIn << std::endl;
					std::cout << "        idOut   : " << (*it).idOut << std::endl;
					std::cout << "        portOut : " << (*it).portOut << std::endl;
				}
				std::cout << "End Connection list" << std::endl;
			#endif

			do
			{
				for(int i=0; i<tmpConnections.size(); i++)
				{
					if(tmpConnections[i].idIn>startPipeline)
					{
						int merges = 0;
						for(int j=0; j<tmpConnections.size(); j++)
						{
							if(tmpConnections[i].idIn==tmpConnections[j].idOut && tmpConnections[i].portIn==tmpConnections[j].portOut && connectionOwner[i]!=connectionOwner[j])
							{
								#ifdef __DEVELOPMENT_VERBOSE__
									std::cout << "    Merging connection : " << std::endl;
									std::cout << "        idIn    : " << tmpConnections[i].idIn 	<< "\t idIn    : " << tmpConnections[j].idIn 	<< std::endl;
									std::cout << "        portIn  : " << tmpConnections[i].portIn 	<< "\t portIn  : " << tmpConnections[j].portIn 	<< std::endl;
									std::cout << "        idOut   : " << tmpConnections[i].idOut 	<< "\t idOut   : " << tmpConnections[j].idOut 	<< std::endl;
									std::cout << "        portOut : " << tmpConnections[i].portOut 	<< "\t portOut : " << tmpConnections[j].portOut << std::endl;
								#endif
								Connection c;
								c.idIn 		= tmpConnections[j].idIn;
								c.portIn 	= tmpConnections[j].portIn;
								c.idOut 	= tmpConnections[i].idOut;
								c.portOut 	= tmpConnections[i].portOut;

								#ifdef __DEVELOPMENT_VERBOSE__
									if(j<i)
									{
										std::cout << "    Comparison Connection BEFORE : " << std::endl;
										std::cout << "        idIn    : " << tmpConnections[i].idIn << std::endl;
										std::cout << "        portIn  : " << tmpConnections[i].portIn << std::endl;
										std::cout << "        idOut   : " << tmpConnections[i].idOut << std::endl;
										std::cout << "        portOut : " << tmpConnections[i].portOut << std::endl;
									}
								#endif
								tmpConnections.erase(tmpConnections.begin() + j);
								connectionOwner.erase(connectionOwner.begin() + j);
								if(j<i)
								{
									i--;
									#ifdef __DEVELOPMENT_VERBOSE__
										std::cout << "    Comparison Connection AFTER : " << std::endl;
										std::cout << "        idIn    : " << tmpConnections[i].idIn << std::endl;
										std::cout << "        portIn  : " << tmpConnections[i].portIn << std::endl;
										std::cout << "        idOut   : " << tmpConnections[i].idOut << std::endl;
										std::cout << "        portOut : " << tmpConnections[i].portOut << std::endl;
									#endif
								}
								j--;
								tmpConnections.push_back(c);
								merges++;
							}
						}
						if(merges>0)
						{
							tmpConnections.erase(tmpConnections.begin() + i);
							connectionOwner.erase(connectionOwner.begin() + i);
						}
					}
				}

				// Is there any modification to do?
				test = false;
				for(TableConnection::iterator it=tmpConnections.begin(); it!=tmpConnections.end() && !test; it++)
					if((*it).idOut>startPipeline)
					{
						#ifdef __DEVELOPMENT_VERBOSE__
							std::cout << "Modifications remain" << std::endl;
							std::cout << "        idIn    : " << (*it).idIn << std::endl;
							std::cout << "        portIn  : " << (*it).portIn << std::endl;
							std::cout << "        idOut   : " << (*it).idOut << std::endl;
							std::cout << "        portOut : " << (*it).portOut << std::endl;
						#endif
						test = true;
						throw Exception("Stop");
					}
			} while(test);

			#ifdef __VERBOSE__
				std::cout << "Connection list : " << tmpConnections.size() << std::endl;
				for(TableConnection::iterator it=tmpConnections.begin(); it!=tmpConnections.end(); it++)
				{
					std::cout << "    Connection  : " << std::endl;
					std::cout << "        idIn    : " << (*it).idIn << std::endl;
					std::cout << "        portIn  : " << (*it).portIn << std::endl;
					std::cout << "        idOut   : " << (*it).idOut << std::endl;
					std::cout << "        portOut : " << (*it).portOut << std::endl;
				}
				std::cout << "End Connection list" << std::endl;
			#endif

			// 3nd Step

			// Manage memory
			#ifdef __DEVELOPMENT_VERBOSE__
				std::cout << "Third step" << std::endl;
			#endif

			std::vector<int> 	availabilty;
			TableConnection		remainingConnections = tmpConnections;
			TableIndex		options;

			// Prepare the buffer list :
			buffers.assign(filters.size(), NULL);
			listOfArgBuffers.assign(filters.size(), NULL);
			listOfArgBuffersOutput.assign(filters.size(), NULL);

			//buffers[10] = NULL; // ?????????????????????

			#ifdef __DEVELOPMENT_VERBOSE__
				std::cout << "Updating availability" << std::endl;
			#endif

			// Update availability :
			for(TableFilter::iterator it=filters.begin(); it!=filters.end(); it++)
				availabilty.push_back((*it)->getNumInputPort());

			#ifdef __DEVELOPMENT_VERBOSE__
				std::cout << "Updating first links" << std::endl;
			#endif
			// Update the first links :
			for(TableConnection::iterator it=remainingConnections.begin(); it!=remainingConnections.end(); it++)
			{
				if((*it).idOut==THIS_PIPELINE)
				{
					availabilty[(*it).idIn]--;
					remainingConnections.erase(it);
					it--;
				}
			}

			#ifdef __DEVELOPMENT_VERBOSE__
				std::cout << "Starting decision loop" << std::endl;
			#endif
			bool remaingFilter 	= false;
			bool stuck 		= true;
			do
			{
				remaingFilter 	= false;
				stuck 		= (options.size()==0);

				// Put the available filters in options
				#ifdef __DEVELOPMENT_VERBOSE__
					std::cout << "    Options list contains " << options.size() << " elements" << std::endl;
				#endif
				for(int id=0; id<availabilty.size(); id++)
				{
					if(availabilty[id]==0)
					{
						#ifdef __DEVELOPMENT_VERBOSE__
							std::cout << "        adding filter " << id << " to the option list" << std::endl;
						#endif
						availabilty[id] = -1;
						options.push_back(id);
						stuck = false;
					}
					else
					{
						if(availabilty[id]>0)
						{
							#ifdef __DEVELOPMENT_VERBOSE__
								std::cout << "        discarding filter " << id << " of the option list : " << availabilty[id] << std::endl;
							#endif
							remaingFilter = true;
						}
					}
				}

				if(stuck && remaingFilter)
					throw Exception("Pipeline::build - Error : Building routine is stuck probably because of missing connection.", __FILE__, __LINE__);

				#ifdef __DEVELOPMENT_VERBOSE__
					std::cout << "    Best Filter" << std::endl;
				#endif
				// Search the best next filter in 'options'
				TableIndex::iterator bestIt = options.begin();
				bool notFoundMandatory = true;
				int size = std::numeric_limits<int>::max();

				for(TableIndex::iterator it=options.begin(); it!=options.end() && notFoundMandatory; it++)
				{
					#ifdef __DEVELOPMENT_VERBOSE__
						std::cout << "        Monitoring : " << *it << '/' << filters.size() << std::endl;
						std::cout << "        Name       : " << filters[(*it)]->getName() << std::endl; //SEGFAULT if extended
					#endif
					// Is it a mandatory step?
						// Look for the format in the FBO list
						bool alreadyExist = false;
						for(TableBuffer::iterator it2=buffers.begin(); it2!=buffers.end() && !alreadyExist; it2++)
						{
							if((*it2)!=NULL)
								// If same texture format between the FBO and the target filter :
								if((*(*it2))==(*filters[*it]) && (*it2)->getAttachmentCount()==filters[*it]->getNumOutputPort())
									alreadyExist = true;
						}

					if(alreadyExist)
					{
						// Compare the size with the best one found for the moment :
						int s = filters[*it]->getSize();
						if(size>s)
						{
							size = s;
							bestIt = it;
						}
					}
					else // If not alreadyExist, hen it is mandatory to create it :
					{
						notFoundMandatory = false;
						bestIt = it;
					}
				}

				#ifdef __DEVELOPMENT_VERBOSE__
					std::cout << "    Saving Best Filter" << std::endl;
				#endif

				// Remove it from the options :
				int best = *bestIt;
				options.erase(bestIt);

				// Create the FBO :
				buffers[best] = new HdlFBO(*filters[best], filters[best]->getNumOutputPort());
				#ifdef __DEVELOPMENT_VERBOSE__
					std::cout << "Creating : " << (*buffers[best])[0] << std::endl;
				#endif

				// Save the corresponding action in the action list :
				actionFilter.push_back(best);

				// Create the argument list :
				TableIndex 	*bufferArg = new TableIndex,
						*outputArg = new TableIndex;
				bufferArg->resize(filters[best]->getNumInputPort());
				outputArg->resize(filters[best]->getNumInputPort());

				// Find in the connections, the input parts
				for(TableConnection::iterator it=tmpConnections.begin(); it!=tmpConnections.end(); it++)
				{
					if((*it).idIn==best)
					{
						test++;
						(*bufferArg)[(*it).portIn] = (*it).idOut;
						(*outputArg)[(*it).portIn] = (*it).portOut;
						#ifdef __DEVELOPMENT_VERBOSE__
							std::cout << "    Connecting port " << (*it).portIn << " to buffer " << (*it).idOut << "::" << (*it).portOut << std::endl;
						#endif
					}

				}

				listOfArgBuffers[best] 		= bufferArg;
				listOfArgBuffersOutput[best] 	= outputArg;

				#ifdef __DEVELOPMENT_VERBOSE__
					std::cout << "    Removing connection outgoing from best filter" << std::endl;
				#endif
				// Find in the connections, the filters that depends on this step
				for(TableConnection::iterator it=remainingConnections.begin(); it!=remainingConnections.end(); it++)
				{
					if((*it).idOut==best && (*it).idIn!=THIS_PIPELINE) // find a filter that is not this!
					{
						availabilty[(*it).idIn]--;
						remainingConnections.erase(it);
						it--;
					}
				}
			}
			while( remaingFilter );

			#ifdef __DEVELOPMENT_VERBOSE__
				std::cout << "End decision loop" << std::endl;
			#endif

			// Check if all filters are in :
			for(std::vector<int>::iterator it=availabilty.begin(); it!=availabilty.end(); it++)
				if((*it)!=-1)
					throw Exception("Pipeline::build - Error : some filters aren't connected at the end of the parsing step in pipeline " + getNameExtended(), __FILE__, __LINE__);

			// Add the coordinates of the output buffers :
			outputBuffer.assign(getNumOutputPort(), 0);
			outputBufferPort.assign(getNumOutputPort(), 0);
			for(TableConnection::iterator it=tmpConnections.begin(); it!=tmpConnections.end(); it++)
			{
				if((*it).idIn==THIS_PIPELINE)
				{
					outputBuffer[(*it).portIn] 	= (*it).idOut;
					outputBufferPort[(*it).portIn] 	= (*it).portOut;
				}
			}

			// Print the final layout :
			#ifdef __VERBOSE__
				std::cout << "Actions : " << std::endl;
				for(int i=0; i<actionFilter.size(); i++)
				{
					std::cout << "    Action " << i+1 << '/' << actionFilter.size() << " -> Filter : <" << actionFilter[i] << "> " << filters[actionFilter[i]]->getName() << std::endl;
					for(int j=0; j<listOfArgBuffers[actionFilter[i]]->size(); j++)
						std::cout << "        Connection " << j << " to : (" << (*listOfArgBuffers[actionFilter[i]])[j] << ';' << (*listOfArgBuffersOutput[actionFilter[i]])[j] << ')' << std::endl;
				}
				std::cout << "End Actions" << std::endl;
			#endif
		}
		catch(std::exception& e)
		{
			Exception m("Pipeline::build - Error while building the pipeline " + getNameExtended(), __FILE__, __LINE__);
			throw m+e;
		}
	}

	/**
	\fn int Pipeline::getSize(void)
	\brief Get the size in bytes of the elements on the GPU for this pipeline.
	\return Size in bytes.
	**/
	int Pipeline::getSize(void)
	{
		int size = 0;

		for(TableBuffer::iterator it = buffers.begin(); it!=buffers.end(); it++)
			size += (*it)->getSize();

		return size;
	}

	/**
	\fn void Pipeline::process(void)
	\brief Apply the pipeline.
	**/
	void Pipeline::process(void)
	{
		clock_t 	timing,
				totalTiming = clock();

		#ifdef __DEVELOPMENT_VERBOSE__
			std::cout << "Processing : " << getNameExtended() << std::endl;
		#endif

		for(int i = 0; i<actionFilter.size(); i++)
		{
			int action = actionFilter[i];

			#ifdef __DEVELOPMENT_VERBOSE__
				std::cout << "    applying filter : " << filters[action]->getNameExtended() << "..." << std::endl;
			#endif

			// Apply filter *filter[actionFilter]
			Filter* f = filters[action];
			for(int j=0; j<f->getNumInputPort(); j++)
			{
				int bufferID 	= (*listOfArgBuffers[action])[j];
				int portID 	= (*listOfArgBuffersOutput[action])[j];

				#ifdef __DEVELOPMENT_VERBOSE__
					std::cout << "        conecting buffer " << bufferID << " on port " << portID << std::endl;
				#endif

				if(bufferID==THIS_PIPELINE)
					f->setInputForNextRendering(j, input[portID]);
				else
					f->setInputForNextRendering(j, (*buffers[bufferID])[portID]);
			}

			#ifdef __DEVELOPMENT_VERBOSE__
				std::cout << "        Processing..." << std::endl;
			#endif

			if(perfsMonitoring)
			{
				timing = clock();
				glFlush();
			}

			f->process(*buffers[action]);

			if(perfsMonitoring)
			{
				glFlush();
				timing = clock() - timing;
				perfs[action] = static_cast<float>(timing)/static_cast<float>(CLOCKS_PER_SEC)*1000.0f;
			}

			#ifdef __DEVELOPMENT_VERBOSE__
				std::cout << "        Done." << std::endl;
			#endif
		}

		if(perfsMonitoring)
		{
			totalTiming = clock() - totalTiming;
			totalPerf   = static_cast<float>(totalTiming)/static_cast<float>(CLOCKS_PER_SEC)*1000.0f;
		}
	}

	/**
	\fn Pipeline& Pipeline::operator<<(HdlTexture& t)
	\brief Add a data as input to the pipeline.
	\param t The data to use.
	\return This pipeline or raise an exception if any errors occur.
	**/
	Pipeline& Pipeline::operator<<(HdlTexture& t)
	{
		if(input.size()>=getNumInputPort())
			throw Exception("Pipeline::operator<<(HdlTexture&) - Too much arguments given to Pipeline " + getNameExtended(), __FILE__, __LINE__);

		input.push_back(&t);

		return *this;
	}

	/**
	\fn Pipeline& Pipeline::operator<<(ActionType a)
	\brief Apply operation on previously input data.
	\param a The ActionType (Process or Reset arguments).
	\return This pipeline or raise an exception if any errors occur.
	**/
	Pipeline& Pipeline::operator<<(ActionType a)
	{
		// Check the number of arguments given :
		if(input.size()!=getNumInputPort())
			throw Exception("Pipeline::operator<<(HdlTexture&) - Too few arguments given to Pipeline " + getNameExtended(), __FILE__, __LINE__);

		switch(a)
		{
			case Process:
				process();
			case Reset:            // After Process do Reset of the Input
				cleanInput();
				break;
			default:
				throw Exception("Pipeline::operator<<(ActionType) - Unknown action for Pipeline " + getNameExtended(), __FILE__, __LINE__);
		}

		return *this;
	}

	/**
	\fn HdlTexture& Pipeline::out(int i)
	\brief Return the output of the pipeline.
	\param i The ID of the output port.
	\return A reference to the corresponding output texture or raise an exception if any errors occur.
	**/
	HdlTexture& Pipeline::out(int i)
	{
		checkOutputPort(i);
		int bufferID 		= outputBuffer[i];
		int bufferPortID 	= outputBufferPort[i];
		return *((*buffers[bufferID])[bufferPortID]);
	}

	/**
	\fn HdlTexture& Pipeline::out(const std::string& portName)
	\brief Return the output of the pipeline.
	\param portName The name of the output port.
	\return A reference to the corresponding output texture or raise an exception if any errors occur.
	**/
	HdlTexture& Pipeline::out(const std::string& portName)
	{
		int index = getInputPortID(portName);
		return out(index);
	}

	/**
	\fn Filter& Pipeline::operator[](const std::string& name)
	\brief Access to the filter described by the path.
	\param name The path.
	\return A reference to the corresponding filter instance or raise an exception if any errors occur.
	**/
	int Pipeline::getFilterID(const std::string& path)
	{
		try
		{
			__ReadOnly_PipelineLayout* p = this;

			// Parse the identification path and return a filter if so
			std::vector<std::string> tree = ObjectName::parse(path);

			std::string filter = tree.back();
			tree.pop_back();

			for(std::vector<std::string>::iterator it=tree.begin(); it!=tree.end(); it++)
			{
				__ReadOnly_PipelineLayout& tmp = p->pipelineLayout(*it);
				p = &tmp;
			}

			int id = p->getElementIndex(filter);

			if(p->getElementKind(id)!=FILTER)
				throw Exception("Pipeline::operator[] - The element " + path + " isn't a filter", __FILE__, __LINE__);

			if(p->getElementID(id)==ELEMENT_NOT_ASSOCIATED)
				throw Exception("Pipeline::operator[] - The element " + path + " exists but wasn't associated (internal error)", __FILE__, __LINE__);

			return p->getElementID(id);
		}
		catch(std::exception& e)
		{
			Exception m("Pipeline::getFilterID - Error while processing request on " + path, __FILE__, __LINE__);
			throw m+e;
		}
	}

	/**
	\fn Filter& Pipeline::operator[](const std::string& name)
	\brief Access to the filter described by the path.
	\param name The path.
	\return A reference to the corresponding filter instance or raise an exception if any errors occur.
	**/
	Filter& Pipeline::operator[](const std::string& path)
	{
		try
		{
			return *filters[getFilterID(path)];
		}
		catch(std::exception& e)
		{
			Exception m("Pipeline::operator[] - Error while processing request on " + path, __FILE__, __LINE__);
			throw m+e;
		}
	}

	/**
	\fn void Pipeline::enablePerfsMonitoring(void)
	\brief Enable performances monitoring.
	**/
	void Pipeline::enablePerfsMonitoring(void)
	{
		if(!perfsMonitoring)
		{
			perfsMonitoring = true;
			perfs.assign(filters.size(),0.0f);
			totalPerf = 0.0f;
		}
	}

	/**
	\fn void Pipeline::disablePerfsMonitoring(void)
	\brief Disable performances monitoring.
	**/
	void Pipeline::disablePerfsMonitoring(void)
	{
		if(perfsMonitoring)
		{
			perfsMonitoring = false;
			perfs.clear();
			totalPerf = 0.0;
		}
	}

	/**
	\fn float Pipeline::getTiming(const std::string& path)
	\brief Get last result of performance monitoring IF it is still enabled.
	\param path The path to the filter.
	\return Time in milliseconds needed to apply the filter (not counting bonding operation).
	**/
	float Pipeline::getTiming(const std::string& path)
	{
		if(perfsMonitoring)
			return perfs[getFilterID(path)];
		else
			throw Exception("Pipeline::getTiming - Monitoring is disabled.", __FILE__, __LINE__);
	}

	/**
	\fn float Pipeline::getTotalTiming(void)
	\brief Get total time ellapsed for last run.
	\return Time in milliseconds needed to apply the whole pipeline (counting everything and flushing after each filter).
	**/
	float Pipeline::getTotalTiming(void)
	{
		if(perfsMonitoring)
			return totalPerf;
		else
			throw Exception("Pipeline::getTotalTiming - Monitoring is disabled.", __FILE__, __LINE__);
	}
