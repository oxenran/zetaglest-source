//
//      ai_interface.h:
//
//      This file is part of ZetaGlest <https://github.com/ZetaGlest>
//
//      Copyright (C) 2018  The ZetaGlest team
//
//      ZetaGlest is a fork of MegaGlest <https://megaglest.org>
//
//      This program is free software: you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation, either version 3 of the License, or
//      (at your option) any later version.

//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program.  If not, see <https://www.gnu.org/licenses/>

#ifndef _GLEST_GAME_AIINTERFACE_H_
#   define _GLEST_GAME_AIINTERFACE_H_

#   include "world.h"
#   include "commander.h"
#   include "command.h"
#   include "conversion.h"
#   include "ai.h"
#   include "game_settings.h"
#   include <map>
#   include "leak_dumper.h"

using
Shared::Util::intToStr;

namespace
	Glest {
	namespace
		Game {

		// =====================================================
		//      class AiInterface
		//
		///     The AI will interact with the game through this interface
		// =====================================================

		class
			AiInterfaceThread :
			public
			BaseThread,
			public
			SlaveThreadControllerInterface {
		protected:

			AiInterface *
				aiIntf;
			Semaphore
				semTaskSignalled;
			Mutex *
				triggerIdMutex;
			std::pair < int,
				bool >
				frameIndex;
			MasterSlaveThreadController *
				masterController;

			virtual void
				setQuitStatus(bool value);
			virtual void
				setTaskCompleted(int frameIndex);

		public:
			explicit
				AiInterfaceThread(AiInterface * aiIntf);
			virtual ~
				AiInterfaceThread();
			virtual void
				execute();
			void
				signal(int frameIndex);
			bool
				isSignalCompleted(int frameIndex);

			virtual void
				setMasterController(MasterSlaveThreadController * master) {
				masterController = master;
			}
			virtual void
				signalSlave(void *userdata) {
				signal(*((int *) (userdata)));
			}

			virtual void
				signalQuit();
			virtual bool
				canShutdown(bool deleteSelfIfShutdownDelayed = false);
		};

		class
			AiInterface {
		private:
			World *
				world;
			Commander *
				commander;
			Console *
				console;
			GameSettings *
				gameSettings;

			Ai
				ai;

			int
				timer;
			int
				factionIndex;
			int
				teamIndex;

			//config
			bool
				redir;
			int
				logLevel;
			std::string
				aiLogFile;
			FILE *
				fp;

			std::map < const ResourceType *, int >
				cacheUnitHarvestResourceLookup;

			Mutex *
				aiMutex;

			AiInterfaceThread *
				workerThread;
			std::vector <
				Vec2i >
				enemyWarningPositionList;

		public:
			AiInterface(Game & game, int factionIndex, int teamIndex,
				int useStartLocation = -1);
			~
				AiInterface();

			AiInterface(const AiInterface & obj) {
				init();
				throw
					megaglest_runtime_error("class AiInterface is NOT safe to copy!");
			}
			AiInterface &
				operator= (const AiInterface & obj) {
				init();
				throw
					megaglest_runtime_error("class AiInterface is NOT safe to assign!");
			}

			//main
			void
				update();

			std::vector < Vec2i > getEnemyWarningPositionList()const {
				return
					enemyWarningPositionList;
			}
			void
				removeEnemyWarningPositionFromList(Vec2i & checkPos);

			inline Mutex *
				getMutex() {
				return aiMutex;
			}

			void
				signalWorkerThread(int frameIndex);
			bool
				isWorkerThreadSignalCompleted(int frameIndex);
			AiInterfaceThread *
				getWorkerThread() {
				return workerThread;
			}

			bool
				isLogLevelEnabled(int level);

			//get
			int
				getTimer() const {
				return
					timer;
			}
			int
				getFactionIndex() const {
				return
					factionIndex;
			}

			//misc
			void
				printLog(int logLevel, const string & s);

			//interact
			std::pair < CommandResult, string > giveCommand(int unitIndex,
				CommandClass
				commandClass,
				const Vec2i & pos =
				Vec2i(0));
			std::pair < CommandResult, string > giveCommand(int unitIndex,
				const CommandType *
				commandType,
				const Vec2i & pos,
				const UnitType *
				unitType);
			std::pair < CommandResult, string > giveCommand(int unitIndex,
				const CommandType *
				commandType,
				const Vec2i & pos,
				int
				unitGroupCommandId);
			std::pair < CommandResult, string > giveCommand(int unitIndex,
				const CommandType *
				commandType, Unit * u =
				NULL);
			std::pair < CommandResult, string > giveCommand(const Unit * unit,
				const CommandType *
				commandType,
				const Vec2i & pos,
				int
				unitGroupCommandId);

			std::pair < CommandResult,
				string > giveCommandSwitchTeamVote(const Faction * faction,
					SwitchTeamVote * vote);

			//get data
			const ControlType
				getControlType();
			int
				getMapMaxPlayers();
			Vec2i
				getHomeLocation();
			Vec2i
				getStartLocation(int locationIndex);
			int
				getFactionCount();
			int
				getMyUnitCount() const;
			int
				getMyUpgradeCount() const;
			//int onSightUnitCount();
			const Resource *
				getResource(const ResourceType * rt);
			const Unit *
				getMyUnit(int unitIndex);
			Unit *
				getMyUnitPtr(int unitIndex);
			//const Unit *getOnSightUnit(int unitIndex);
			const FactionType *
				getMyFactionType();
			Faction *
				getMyFaction();
			const TechTree *
				getTechTree();
			bool
				isResourceInRegion(const Vec2i & pos, const ResourceType * rt,
					Vec2i & resourcePos, int range) const;
			bool
				isResourceNear(const Vec2i & pos, const ResourceType * rt,
					Vec2i & resourcePos, Faction * faction,
					bool fallbackToPeersHarvestingSameResource) const;
			bool
				getNearestSightedResource(const ResourceType * rt, const Vec2i & pos,
					Vec2i & resultPos,
					bool usableResourceTypeOnly);
			bool
				isAlly(const Unit * unit) const;
			bool
				isAlly(int factionIndex) const;
			bool
				reqsOk(const RequirableType * rt);
			bool
				reqsOk(const CommandType * ct);
			bool
				checkCosts(const ProducibleType * pt, const CommandType * ct);
			bool
				isFreeCells(const Vec2i & pos, int size, Field field);
			const Unit *
				getFirstOnSightEnemyUnit(Vec2i & pos, Field & field, int radius);
			Map *
				getMap();
			World *
				getWorld() {
				return world;
			}

			bool
				factionUsesResourceType(const FactionType * factionType,
					const ResourceType * rt);

			void
				saveGame(XmlNode * rootNode) const;
			void
				loadGame(const XmlNode * rootNode, Faction * faction);

		private:
			string getLogFilename()const {
				return
					"ai" +
					intToStr(factionIndex) +
					".log";
			}
			bool
				executeCommandOverNetwork();

			void
				init();
		};

	}
}                              //end namespace

#endif
