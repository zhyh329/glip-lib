#include "dataModules.hpp"

// Module
	Module::Module(ControlModule& _masterModule, QWidget* parent)
	 : QWidget(parent), masterModule(&_masterModule)	
	{
		masterModule->addClient(this);
	}

	Module::~Module(void)
	{
		if(masterModule!=NULL)
			masterModule->removeClient(this);
	}
	
	bool Module::pipelineCanBeCreated(void)					{ return true; }
	bool Module::pipelineCanBeComputed(void)				{ return true; }
	bool Module::pipelineInputsCanBeModified(void)				{ return true; }
	bool Module::textureInputCanBeReleased(int portID, int recordID)	{ return true; }
	bool Module::pipelineUniformsCanBeModified(void)			{ return true; }
	bool Module::pipelineCanBeDestroyed(void)				{ return true; }
	bool Module::canBeClosed(void)						{ return true; }

	bool Module::requirePipelineCreation(const std::string& code)
	{
		if(masterModule==NULL)
			return false;
		else
			return masterModule->requirePipelineCreation(code);
	}

	bool Module::requirePipelineComputation(void)
	{
		if(masterModule==NULL)
			return false;
		else
			return masterModule->requirePipelineComputation();
	}

	bool Module::registerInputTexture(int recordID, int portID)
	{
		if(masterModule==NULL)
			return false;
		else
			return masterModule->registerInputTexture(this, recordID, portID);
	}

	void Module::unregisterInputTexture(int recordID)
	{
		if(masterModule!=NULL)
			masterModule->unregisterInputTexture(this, recordID);
	}

	bool Module::requirePrepareToPipelineUniformsModification(void)
	{
		if(masterModule==NULL)
			return false;
		else
			return masterModule->requirePrepareToPipelineUniformsModification();
	}

	bool Module::requirePipelineDestruction(void)
	{
		if(masterModule==NULL)
			return false;
		else
			return masterModule->requirePipelineDestruction();
	}

	bool Module::requireDisplay(WindowRenderer*& display)
	{
		if(masterModule==NULL)
		{
			display = NULL;
			return false;			
		}
		else
			return masterModule->linkToDisplay(this, display);
	}

	void Module::updateDisplay(WindowRenderer& display)
	{
		display.clearWindow();
	}

	bool Module::canReleaseDisplay(void)
	{
		return true;
	}

	void Module::preparePipelineLoading(LayoutLoader& loader, const LayoutLoader::PipelineScriptElements& infos)
	{ }
	
	bool Module::isValidTexture(int recordID)
	{
		return false;
	}

	HdlTexture& Module::getTexture(int recordID)
	{
		throw Exception("Module::getTexture - Module does not have textures.", __FILE__, __LINE__);
	}

	bool Module::pipelineExists(void) const
	{
		if(masterModule==NULL)
			return false;
		else
			return masterModule->pipelineExists();
	}

	bool Module::lastComputationWasSuccessful(void) const
	{
		if(masterModule==NULL)
			return false;
		else
			return masterModule->lastComputationWasSuccessful();
	}

	bool Module::isInputValid(int portID)
	{
		if(masterModule==NULL)
			return false;
		else
			return masterModule->isInputValid(portID);
	}

	HdlTexture& Module::inputTexture(int portID)
	{
		if(masterModule==NULL)
			throw Exception("Module::inputTexture - No master module (internal error).", __FILE__, __LINE__);
		else
			return masterModule->inputTexture(portID);
	}

	const std::string& Module::getPipelineCode(void) const
	{
		if(pipelineExists())
			return masterModule->getPipelineCode();
		else
			return "";
	}
	
	const Pipeline& Module::pipeline(void) const
	{
		if(pipelineExists())
		 	return masterModule->pipeline();

		else
			throw Exception("Module::pipeline - No pipeline defined.", __FILE__, __LINE__);
	}

	bool Module::isThisLinkedToDisplay(void) const
	{
		if(masterModule==NULL)
			return false;
		else
			return masterModule->testLinkToDisplay(this);
	}

	Pipeline& Module::pipeline(void)
	{
		if(pipelineExists())
			return masterModule->pipeline();
		else
			throw Exception("Module::pipeline - No pipeline defined.", __FILE__, __LINE__);
	}

	void Module::pipelineWasCreated(void)			{}
	void Module::pipelineCompilationFailed(Exception& e)	{}
	void Module::pipelineWasComputed(void)			{}
	void Module::pipelineComputationFailed(Exception& e)	{}
	void Module::pipelineInputWasModified(int portID)	{}
	void Module::pipelineInputWasReleased(int portID)	{}
	void Module::pipelineUniformsWereModified(void)		{}
	void Module::pipelineWasDestroyed(void)			{}

// ControlModule
	const int ControlModule::maxNumInputs = 256;

	ControlModule::ControlModule(QWidget* parent)
	 : QWidget(parent), display(this, 640, 480), lastComputationSucceeded(false), pipelinePtr(NULL), displayClient(NULL)
	{
		QObject::connect(&(display.renderer()),	SIGNAL(actionReceived(void)), this, SLOT(displayUpdate(void)));

		LayoutLoaderModule::addBasicModules(pipelineLoader);

		inputTextureRecordIDs.assign(maxNumInputs, -1);
		inputTextureOwners.assign(maxNumInputs, NULL);
	}

	ControlModule::~ControlModule(void)
	{
		for(std::vector<Module*>::iterator it=clients.begin(); it!=clients.end(); it++)
			(*it)->masterModule = NULL;		

		clients.clear();

		delete pipelinePtr;
		pipelinePtr = NULL;
		pipelineCode.clear();
		displayClient = NULL;
	}

	// Private slots : 
	void ControlModule::displayUpdate(void)
	{
		if(displayClient==NULL)
			display.renderer().clearWindow();
		else
			displayClient->updateDisplay( display.renderer() );
	}

	// Tools : 
	void ControlModule::addClient(Module* m)
	{
		// Push to the list : 
		clients.push_back(m);

		// Connect the outputs : 
		QObject::connect( this, SIGNAL(pipelineWasCreated()), 			m, SLOT(pipelineWasCreated()) );
		QObject::connect( this, SIGNAL(pipelineCompilationFailed(Exception&)),	m, SLOT(pipelineCompilationFailed(Exception&)) );
		QObject::connect( this, SIGNAL(pipelineWasComputed()), 			m, SLOT(pipelineWasComputed()) );
		QObject::connect( this, SIGNAL(pipelineComputationFailed(Exception&)),	m, SLOT(pipelineComputationFailed(Exception&)) );
		QObject::connect( this, SIGNAL(pipelineInputWasModified(int)), 		m, SLOT(pipelineInputWasModified(int)) );
		QObject::connect( this, SIGNAL(pipelineInputWasReleased(int)), 		m, SLOT(pipelineInputWasReleased(int)) );
		QObject::connect( this, SIGNAL(pipelineUniformsWereModified()), 	m, SLOT(pipelineUniformsWereModified()) );
		QObject::connect( this, SIGNAL(pipelineWasDestroyed()), 		m, SLOT(pipelineWasDestroyed()) );

		// Connect the inputs :
		QObject::connect( m, SIGNAL(pipelineUniformModification()),		this, SLOT(pipelineUniformModification()) );
	}

	void ControlModule::removeClient(Module* m)
	{
		std::vector<Module*>::iterator it = std::find(clients.begin(), clients.end(), m);

		if(it!=clients.end())
			clients.erase(it);

		releaseDisplayLink(m);
	}
	
	bool ControlModule::linkToDisplay(Module* m, WindowRenderer*& displayPtr)
	{
		if(displayClient!=NULL)
		{
			if(displayClient->canReleaseDisplay())
			{
				displayClient = m;
				displayPtr = &display.renderer();
				return true;
			}
			else
			{
				displayPtr = NULL;
				return false;
			}
		}
		else
		{
			displayClient = m;
			displayPtr = &display.renderer();
			return true;
		}
	}

	bool ControlModule::testLinkToDisplay(const Module* m) const
	{
		return displayClient==m;
	}

	void ControlModule::releaseDisplayLink(Module* m)
	{
		if(displayClient==m)
		{
			displayClient = NULL;
			displayUpdate();
		}
	}

	bool ControlModule::pipelineExists(void) const
	{
		return pipelinePtr!=NULL;
	}

	bool ControlModule::lastComputationWasSuccessful(void) const
	{
		return pipelineExists() && lastComputationSucceeded;
	}
	
	bool ControlModule::isInputValid(int portID)
	{
		if(portID<0 || portID>inputTextureOwners.size())
			return false;
		else if(inputTextureOwners[portID]!=NULL)
			return inputTextureOwners[portID]->isValidTexture( inputTextureRecordIDs[portID] );
		else
			return false;
	}

	HdlTexture& ControlModule::inputTexture(int portID)
	{
		if(inputTextureOwners[portID]==NULL)
			throw Exception("ControlModule::inputTexture - Input texture " + to_string(portID) + " does not have owner (internal error).", __FILE__, __LINE__);
		else
			return inputTextureOwners[portID]->getTexture(inputTextureRecordIDs[portID]);
	}

	const std::string& ControlModule::getPipelineCode(void) const
	{
		if(pipelineExists())
			return pipelineCode;
		else
			return "";
	}

	const Pipeline& ControlModule::pipeline(void) const
	{
		if(pipelineExists())
			return *pipelinePtr;
		else
			throw Exception("ControlModule::pipeline - No pipeline in use.", __FILE__, __LINE__);
	}

	Pipeline& ControlModule::pipeline(void)
	{
		if(pipelineExists())
			return *pipelinePtr;
		else
			throw Exception("ControlModule::pipeline - No pipeline in use.", __FILE__, __LINE__);
	}

	bool ControlModule::requirePipelineCreation(const std::string& code)
	{
		// First, is there a removal : 
		if(pipelineExists())
		{
			if(!requirePipelineDestruction())
				return false; // If the previous pipeline destruction was aborted...
		}

		bool poll = true;

		for(std::vector<Module*>::iterator it=clients.begin(); it!=clients.end() && poll; it++)
			poll = poll && (*it)->pipelineCanBeCreated();
	
		if(poll)
		{
			// Create the pipeline : 
			try
			{
				// Delete previous pipeline : 
				delete pipelinePtr;
				pipelinePtr			= NULL;
				lastComputationSucceeded	= false;
				
				// Copy the code : 
				pipelineCode 			= code;

				// First step : get informations.
				pipelineLoader.clearRequiredElements();
				LayoutLoader::PipelineScriptElements infos = pipelineLoader.listElements(pipelineCode);	

				// Ask all modules to prepare the loader : 
				for(std::vector<Module*>::iterator it=clients.begin(); it!=clients.end() && poll; it++)
					(*it)->preparePipelineLoading(pipelineLoader, infos);

				// This must also prepare the loader with known formats : 
				for(int k=0; k<infos.mainPipelineInputs.size(); k++)
				{
					if(isInputValid(k))
						pipelineLoader.addRequiredElement( infos.mainPipelineInputs[k], inputTexture(k).format() );
					else
						pipelineLoader.addRequiredElement( infos.mainPipelineInputs[k], HdlTextureFormat(1, 1, GL_RGB, GL_UNSIGNED_BYTE) ); // non blocking
				}

				// Compile :
				pipelinePtr = pipelineLoader(pipelineCode, "");

				// Update the bits : 
				lastComputationSucceeded	= true;

				// Propagate the information : 
				emit pipelineWasCreated();
			}
			catch(Exception& e)
			{
				// Manage exception
				emit pipelineCompilationFailed(e);

				// Clear out : 
				delete pipelinePtr;
				pipelinePtr			= NULL;
				pipelineCode.clear();
			}

			return true;
		}
		else
			return false;
	}

	bool ControlModule::pipelineComputation(void)
	{
		if(!pipelineExists())
		{
			lastComputationSucceeded = false;
			return false;
		}

		lastComputationSucceeded = true;

		try
		{
			// Clean : 
			pipeline() << Pipeline::Reset;

			// Check that all inputs are present :
			bool greenLight = true; 
			for(int k=0; k<pipeline().getNumInputPort(); k++)
			{
				if(!isInputValid(k))
				{
					greenLight = false;
					break;
				}	
				else
					pipeline() << inputTexture(k);
			}

			if(!greenLight)
				lastComputationSucceeded = false;
			else
			{
				// Compute : 
				pipeline() << Pipeline::Process;			
	
				// Signals : 
				emit pipelineWasComputed();
			}
		}
		catch(Exception& e)
		{
			emit pipelineComputationFailed(e);
			lastComputationSucceeded = false;
		}

		return lastComputationSucceeded;
	}

	bool ControlModule::requirePipelineComputation(void)
	{
		bool poll = true;

		for(std::vector<Module*>::iterator it=clients.begin(); it!=clients.end() && poll; it++)
			poll = poll && (*it)->pipelineCanBeComputed();

		if(poll)
			return pipelineComputation();
		else
			return false;
	}

	bool ControlModule::registerInputTexture(Module* m, int recordID, int portID)
	{
		if(portID<0 || portID>=maxNumInputs)
			throw Exception("ControlModule::registerInputTexture - Port ID " + to_string(portID) + "is out of bounds ([0; " + to_string(maxNumInputs-1) + "]).", __FILE__, __LINE__);

		// Check if all modules allow inputs change : 
		bool poll = true;

		for(std::vector<Module*>::iterator it=clients.begin(); it!=clients.end() && poll; it++)
			poll = poll && (*it)->pipelineInputsCanBeModified();

		if( poll )
		{
			if(inputTextureOwners[portID]!=NULL)
			{
				if(!inputTextureOwners[portID]->textureInputCanBeReleased(portID, inputTextureRecordIDs[portID]))
					return false;
			}
		
			// Else :
			inputTextureRecordIDs[portID] 	= recordID;
			inputTextureOwners[portID]	= m;

			emit pipelineInputWasModified(portID);

			if( pipelineExists() )
				requirePipelineComputation();

			return true;
		}
		else 		
			return false;
	}

	void ControlModule::unregisterInputTexture(Module* m, int recordID)
	{
		int k=0;

		for(k=0; k<inputTextureRecordIDs.size(); k++)
		{
			if(inputTextureRecordIDs[k]==recordID && inputTextureOwners[k]==m)
				break;
		}
		
		if(k<inputTextureRecordIDs.size())
		{
			// Check if all modules allow inputs change : 
			bool poll = true;

			for(std::vector<Module*>::iterator itr=clients.begin(); itr!=clients.end() && poll; itr++)
				poll = poll && (*itr)->pipelineInputsCanBeModified();

			if( poll )
			{
				inputTextureRecordIDs[k]	= -1;
				inputTextureOwners[k]		= NULL;

				emit pipelineInputWasReleased(k);
			}
		}
	}

	bool ControlModule::requirePrepareToPipelineUniformsModification(void)
	{
		bool poll = true;

		for(std::vector<Module*>::iterator it=clients.begin(); it!=clients.end() && poll; it++)
			poll = poll && (*it)->pipelineUniformsCanBeModified();

		return poll;
	}

	bool ControlModule::requirePipelineDestruction(void)
	{
		bool poll = true;

		for(std::vector<Module*>::iterator it=clients.begin(); it!=clients.end() && poll; it++)
			poll = poll && (*it)->pipelineCanBeDestroyed();

		if(poll)
		{
			delete pipelinePtr;
			pipelinePtr 			= NULL;
			lastComputationSucceeded	= false;
			pipelineCode.clear();
			return true;
		}
		else
			return false;
	}

	bool ControlModule::requireClose(void)
	{
		bool poll = true;

		for(std::vector<Module*>::iterator it=clients.begin(); it!=clients.end() && poll; it++)
			poll = poll && (*it)->canBeClosed();

		return poll;
	}

	void ControlModule::pipelineUniformModification(void)
	{
		emit pipelineUniformsWereModified();
	}
