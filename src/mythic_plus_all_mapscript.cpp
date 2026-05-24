/*
 * Credits: silviu20092
 */

#include "ScriptMgr.h"
#include "GameTime.h"
#include "mythic_affix.h"
#include "mythic_plus.h"

class mythic_plus_all_mapscript : public AllMapScript
{
private:
    std::unordered_map<uint32, uint32> instanceTimer;
public:
    mythic_plus_all_mapscript() : AllMapScript("mythic_plus_all_mapscript",
        {
            ALLMAPHOOK_ON_PLAYER_ENTER_ALL,
            ALLMAPHOOK_ON_MAP_UPDATE,
            ALLMAPHOOK_ON_DESTROY_INSTANCE
        })
    {
    }

    void OnPlayerEnterAll(Map* map, Player* player) override
    {
        const MythicPlus::MythicPlusDungeonInfo* savedDungeon = sMythicPlus->GetSavedDungeonInfo(map->GetInstanceId());

        // edge case when players were in a M+ dungeon, server was restarted and the system was disabled in the meantime
        if (!sMythicPlus->IsEnabled() && map->GetInstanceId() != 0)
        {
            if (savedDungeon != nullptr)
            {
                if (savedDungeon->mythicLevel > 0)
                {
                    MythicPlus::BroadcastToPlayer(player, "Попытка присоедениться к сохраненному экземпляру неудачная, т.к. система отключена.");
                    MythicPlus::FallbackTeleport(player);
                    return;
                }
            }
            else
            {
                // save even non Mythic Plus dungeon in DB, this is required for further edge checks
                if (sMythicPlus->MatchMythicPlusMapDiff(map))
                    sMythicPlus->SaveDungeonInfo(map->GetInstanceId(), map->GetId(), 0, 0L, 0, 0, 0, false, false);
            }
        }

        if (sMythicPlus->CanMapBeMythicPlus(map))
        {
            if (savedDungeon != nullptr)
            {
                if (!savedDungeon->isMythic)
                    return; // don't care about regular dungeons

                const MythicLevel* mythicLevel = sMythicPlus->GetMythicLevel(savedDungeon->mythicLevel);
                if (savedDungeon->mythicLevel > 0 && mythicLevel == nullptr)
                {
                    // edge case where a dungeon was saved as M+ but now the level does not longer exist (removed from DB)
                    MythicPlus::BroadcastToPlayer(player, "Это подземелье было сохранено как Мифик+, но теперь его уровня больше не существует.");
                    MythicPlus::FallbackTeleport(player);
                    return;
                }

                if (mythicLevel == nullptr)
                    return;

                if (player->GetLevel() < DEFAULT_MAX_LEVEL)
                {
                    MythicPlus::BroadcastToPlayer(player, "Ваш уровень не соответвует для присоедегнения к Мифик+.");
                    MythicPlus::FallbackTeleport(player);
                    return;
                }

                MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map);
                mapData->mythicPlusStartTimer = savedDungeon->startTime;
                mapData->done = savedDungeon->done;
                mapData->timeLimit = savedDungeon->timeLimit;
                mapData->deaths = savedDungeon->deaths;
                mapData->penaltyOnDeath = savedDungeon->penaltyOnDeath;

                if (!mapData->mythicLevel)
                {
                    MythicPlus::BroadcastToPlayer(player, "Вы только что, вступили в подземелье Мифик+. Все изменения были установлены и сохранены для этого подземелья.");
                    mapData->mythicLevel = mythicLevel;
                }
                else
                    MythicPlus::BroadcastToPlayer(player, "Вы присоеденились к проходящему подземелью Мифик+. Все привязи быфли установлены и активированые для данного подземелья.");

                sMythicPlus->PrintMythicLevelInfo(mapData->mythicLevel, player);

                std::ostringstream oss;
                if (!mapData->done)
                {
                    long long diff = GameTime::GetGameTime().count() - mapData->mythicPlusStartTimer;
                    mapData->updateTimer = diff * 1000;
                    if (diff + mapData->GetPenaltyTime() <= mapData->timeLimit)
                    {
                        oss << "Подземелье с режимом Мифик+ продолжается. Текущий таймер: ";
                        oss << secsToTimeString(diff);
                        oss << ". Уложитесь в отведенное время, чтобы получить награду: ";
                        oss << secsToTimeString(mapData->timeLimit);
                    }
                    else
                    {
                        oss << "Мифик+ сложность для данного подземелья находится в разработке, награда не будет получена. ";
                        oss << "Лимит времени: " << secsToTimeString(mapData->timeLimit);
                        oss << ". Текущий таймер: " << secsToTimeString(diff);
                        mapData->receiveLoot = false;
                    }

                    if (mapData->penaltyOnDeath > 0)
                    {
                        std::ostringstream oss2;
                        oss2 << "Смерть повлечет за собой штраф в размере ";
                        oss2 << secsToTimeString(mapData->penaltyOnDeath);
                        if (mapData->GetPenaltyTime() > 0)
                        {
                            oss2 << ". Текущий штраф: ";
                            oss2 << secsToTimeString(mapData->GetPenaltyTime());

                            oss << ". Текущий штраф: ";
                            oss << secsToTimeString(mapData->GetPenaltyTime());
                        }
                        else
                            oss2 << ". Вы еще не умирали!";
                        MythicPlus::BroadcastToPlayer(player, MythicPlus::Utils::RedColored(oss2.str()));
                    }
                }
                else
                    oss << "Присоеденился к подземелью Мифик+, которое уже пройдено.";
                MythicPlus::AnnounceToPlayer(player, oss.str());
            }
        }
        else
        {
            // if there is a saved dungeon for this non M+ dungeon (as mythic+), it means the server was restarted and now the dungeon
            // is no longer M+ capable
            if (savedDungeon != nullptr && savedDungeon->isMythic)
            {
                MythicPlus::BroadcastToPlayer(player, "Это подземелье сохранено нака Мифик+, теперь оно сброшено и не может быть Мифик+");
                MythicPlus::FallbackTeleport(player);
                return;
            }
        }
    }

    void OnMapUpdate(Map* map, uint32 diff) override
    {
        if (sMythicPlus->IsMapInMythicPlus(map))
        {
            MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map, false);
            ASSERT(mapData);
            if (mapData->receiveLoot && !mapData->done)
            {
                if (mapData->updateTimer / 1000 + mapData->GetPenaltyTime() > mapData->timeLimit)
                {
                    MythicPlus::AnnounceToMap(map, "Время вышло! Вы больше не получите ни какой награды.");
                    mapData->receiveLoot = false;
                }

                mapData->updateTimer += diff;
            }

            for (auto* affix : mapData->mythicLevel->affixes)
                affix->HandlePeriodicEffectMap(map, diff);
        }
        else
        {
            if (sMythicPlus->CanMapBeMythicPlus(map))
            {
                // check if keystone was used
                MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map, false);
                if (mapData != nullptr && mapData->keystoneTimer > 0 && mapData->mythicPlusStartTimer == 0)
                {
                    uint32 instanceId = map->GetInstanceId();

                    if (instanceTimer[instanceId] > MythicPlus::KEYSTONE_START_TIMER)
                    {
    const MythicPlus::MythicPlusDungeonInfo* dsave = sMythicPlus->GetSavedDungeonInfo(instanceId);
    if (dsave != nullptr)
    {
        // на этот момент не должно быть сохранённого подземелья, это значит, что подземелье сохранено как не M+
        return;
    }

    mapData->mythicPlusStartTimer = MythicPlus::Utils::GameTimeCount();

    const MythicLevel* mythicLevel = sMythicPlus->GetMythicLevel(mapData->keystoneLevel);
    ASSERT(mythicLevel);

    std::ostringstream oss;
    oss << "Таймер Mythic Plus запущен! ";
    oss << "Уложитесь в это время, чтобы получить награду: " << secsToTimeString(mythicLevel->timeLimit) << ". ";
    oss << "Приятной игры!";
    MythicPlus::AnnounceToMap(map, oss.str());

    uint32 timeLimit = mythicLevel->timeLimit;
    uint32 mlevel = mythicLevel->level;
    sMythicPlus->SaveDungeonInfo(instanceId, map->GetId(), timeLimit, mapData->mythicPlusStartTimer, mlevel, sMythicPlus->GetPenaltyOnDeath(), 0, false);

    mapData->mythicLevel = mythicLevel;
    mapData->timeLimit = timeLimit;
    mapData->deaths = 0;
    mapData->penaltyOnDeath = sMythicPlus->GetPenaltyOnDeath();

    if (mapData->penaltyOnDeath > 0)
        sMythicPlus->BroadcastToMap(map, MythicPlus::Utils::RedColored("Смерть будет добавлять штраф: " + secsToTimeString(mapData->penaltyOnDeath)));
}
                    else
                        instanceTimer[instanceId] += diff;
                }
            }
        }
    }

    void OnDestroyInstance(MapInstanced* /*mapInstanced*/, Map* map) override
    {
        sMythicPlus->RemoveDungeonInfo(map->GetInstanceId());
    }
};

void AddSC_mythic_plus_all_mapscript()
{
    new mythic_plus_all_mapscript();
}
