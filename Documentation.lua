--Lua Documentation

--Colors-- --Vectors--
{
    Color( red, green, blue )
    Vector2( x, y )
    Vector3( x, y, z )
}

--CmdButtons--
{
	"IN_ATTACK"
	"IN_JUMP"
	"IN_DUCK"
	"IN_FORWARD"
	"IN_BACK"
	"IN_USE"
	"IN_MOVELEFT"
	"IN_MOVERIGHT"
	"IN_ATTACK2"
	"IN_SCORE"
	"IN_BULLRUSH"
}

----------Drawing---------

----------Config----------
config.save("config name")								--Save cheat config
config.load("config name")								--Load cheat config

-------entity_list--------
entity_list.GetHighestEntityIndex()						--Index for last entity
entity_list.GetEntityByIndex( entity_index )			--Entity index, use entity_index as index 

-------globalvars--------
globalvars.curtime()
globalvars.framecount()
globalvars.interval_per_tick()
globalvars.realtime()
globalvars.tickcount()

----------client---------
client.RegisterFunction("hookname", calling_function() ) --Call function in hook
--Hooks--
{
   --on_scene_end           -- For example : drawing chams, bullet impacts, esp drawing
   --on_paint               -- For example : drawing indicators, watermarks, other draw func
   --on_create_move         -- For example : Viewangles changing, antiaim, other Miscellaneous funcs
   --on_frame_stage_notify  -- For example : world color modulation, effects, removals, resolver
}
client.RunScript("lua name")                            --Load LUA script by his name in cheat folder
client.ReloadScripts()                                  --Reload all lua, it is important if you are changed script code
client.Refresh()                                        --Refresh cheat folder, to sync all scripts

----------engine---------
engine.ClientCmd("text")
engine.ClientCmdUnrestricted("text")                    --Do csgo command in CMD system
engine.ExecuteClientCmd("text")
engine.GetLocalPlayerIndex()                            --Get entity index for local player (you)
engine.Maxclients()                                     --Get max clients on server
engine.SendPacket()                                     --Sending packet on server UPD: its using for desync/fakelag functions in cheat
engine.GetPlayerById( userid )                          --Find player by his unique index on server
engine.GetPlayerInfo( userid )                          --Get CSGO player info
engine.GetViewAngles()                                  --Get local player viewangles
engine.SetViewAngles( Angles )                          --Set new local Viewangles/ use angles to set ur angle
engine.IsConnected()                                    --Is local player connected to the server?
engine.IsInGame()                                       --Is local player in game?
engine.GetScreenSize( x,y )                             --Get csgo window size and writing info in variables(x,y)
engine.GetServerTick()                                  --Get Server tick
engine.TakeScreenShot("screenshot name")                --Do valve screenshot and save it by custom name

----------input----------
input.IsHoldingKey( key id )                            --Is key holding right now?
input.IsToogled( key id )                               --Is key toogled?

----------cmd------------
cmd.command_number										--For matching server and client commands for debugging
cmd.tick_count											--The tick the client created this command
cmd.viewangles											--Player instantaneous view angles.
cmd.aimdirection
cmd.forwardmove
cmd.sidemove
cmd.upmove
cmd.buttons												--Attack button states
cmd.impulse
cmd.weaponselect										--Current weapon id
cmd.weaponsubtype
cmd.random_seed											--For shared random functions
cmd.mousedx												--mouse accum in x from create move
cmd.mousedy												--mouse accum in y from create move
cmd.hasbeenpredicted()									--Client only, tracks whether we've predicted this command at least once

----------math-----------
math.CalculateAngle( begin_vector, distance vector )    --Calculate angle by src and distance point
math.ClampValue(variable, minimal_clamp, maximal_clamp )--The function does not allow the value to go beyond the specified limits
math.NormalizeYaw( yaw )                                --Modify yaw to normal return value
math.RandomFloat( min, max )                            --Return random float value
math.RandomInt( min, max )                              --Return random integer value
math.VectorNormalize( Vector )                          --Normalize Vector /// will be updated later
math.WorldToScreen( in__vector, out__vector )           --Transform world vector to window/screen vector
math.FixAngles( vector )                                --Fix angles /// will be updated later
math.Vector_Angles( forward_vector, up_vector,angles )
math.VectorRotate( in_vector, in2_vector )

---------entity---------
entity.IsEnemy( entity )                                --Is entity enemy?
entity.IsDormant( entity )                              --Is entity dormant?
entity.IsAlive( entity )                                --Is entity alive?
entity.GetEntityIndex( entity )                         --Get unique player index
entity.BonePos( entity, bone_id )                       --Get entity bone position, UPD : Use bone_id to find bone
entity.EyePos( entity )                                 --Get entity eye position
entity.IsWeapon( entity )                               --Does entity have weapon?
entity.ActiveWeapon( entity )                           --Get active weapon of entity
entity.GetInaccuracy( entity )                          --Get active weapon inaccuracy  ( also using like hitchance )
entity.GetSpread( entity )                              --Get active weapon spread
entity.GetAbsOrigin( entity )                           --Get entity postition
entity.GetAnimstate( entity )                           --Get entity animation state ( using for animations, resolvers )
entity.IsReloading( entity )                            --Is entity reloading?
entity.AimPunch( entity )                               --Get entity aim punch 
entity.GetHealth( entity )                              --Get entity health value
entity.GetBody( entity )
entity.GetHitboxSet( entity )                           --Get entity hitbox set  /// will be updated later
entity.GetOrigin( entity )                              --Get entity origin
entity.GetAimPunchAngle( entity )                       --Get entity aim punch angle ( using for norecoil )
entity.GetArmorValue( entity )                          --Get entity armor value ( second bar after health in csgo HUD )
entity.CanFire( entity )                                --Can entity shoot?
entity.CanShiftTickBase( entity )                       --Can active weapon shift tickbase ( exploit )  (Using in fastfire exploit, etc)
entity.DuckAmount( entity )
entity.DuckSpeed( entity )
entity.GetFlashDuration( entity )                       --How long player flashed by flashbang grenade
entity.GetCollideable_MAX( entity )                     --Get entity maximal collideable vector (player -  head)
entity.GetCollideable_MIN( entity )                     --Get entity minimal collideable vector (player -  legs)
entity.GetVelocity( entity )                            --Get entity current velocity vector (xyz)
entity.GetVelocitySpeed( entity )                       --Get entity current velocity length (unit)
entity.ZoomLevel( entity )                              --How much scoped entity
entity.HasDefuseKits( entity )                          --Does entity have defuse kits
entity.HasC4( entity )                                  --Does entity have C4
entity.IsScoped( entity )                               --Does entity have Scoped
entity.GetNextAttack( entity )
entity.SimulationTime( entity )
entity.OldSimulationTime( entity )
entity.PlayerName( entity )                             --CSGO Player name in char*
entity.GetShootPos( entity )                            --Entity shoot position
entity.ShotFired( entity )                              --How much shots were did by player
entity.GetTickBase( entity )
entity.GoalFeetYaw( entity )                            
entity.EyeAngles( entity )
entity.m_flCycle( entity )
entity.m_flFeetYawRate( entity )
entity.m_flPlaybackRate( entity )
entity.m_flWeight( entity )
entity.GetSequenceActivity( entity )                    --Check animation layer activity 

-----------cvar----------
cvar.FindVar("ConVar name")                             --Find Convar by his name in valve sdk

----------convar----------
convar.GetInt( convar )                                 --Get integer of convar
convar.GetFloat( convar )                               --Get float of convar
convar.SetInt( convar, new_val )                        --Set integer for convar
convar.SetFloat( convar, new_val )                      --Set float for convar

--KeysID--
VK_LBUTTON        0x01
VK_RBUTTON        0x02
VK_CANCEL         0x03
VK_MBUTTON        0x04   
VK_XBUTTON1       0x05    
VK_XBUTTON2       0x06  
VK_BACK           0x08
VK_TAB            0x09
VK_CLEAR          0x0C
VK_RETURN         0x0D
VK_SHIFT          0x10
VK_CONTROL        0x11
VK_MENU           0x12
VK_PAUSE          0x13
VK_CAPITAL        0x14
VK_KANA           0x15
VK_HANGEUL        0x15  
VK_HANGUL         0x15
VK_JUNJA          0x17
VK_FINAL          0x18
VK_HANJA          0x19
VK_KANJI          0x19
VK_ESCAPE         0x1B
VK_CONVERT        0x1C
VK_NONCONVERT     0x1D
VK_ACCEPT         0x1E
VK_MODECHANGE     0x1F
VK_SPACE          0x20
VK_PRIOR          0x21
VK_NEXT           0x22
VK_END            0x23
VK_HOME           0x24
VK_LEFT           0x25
VK_UP             0x26
VK_RIGHT          0x27
VK_DOWN           0x28
VK_SELECT         0x29
VK_PRINT          0x2A
VK_EXECUTE        0x2B
VK_SNAPSHOT       0x2C
VK_INSERT         0x2D
VK_DELETE         0x2E
VK_HELP           0x2F
VK_0            0x30
VK_1            0x31
VK_2            0x32
VK_3            0x33
VK_4            0x34
VK_5            0x35
VK_6            0x36
VK_7            0x37
VK_8            0x38
VK_9            0x39
VK_A			0x41
VK_B			0x42
VK_C			0x43
VK_D			0x44
VK_E			0x45
VK_F			0x46
VK_G			0x47
VK_H			0x48
VK_I			0x49
VK_J			0x4A
VK_K			0x4B
VK_L			0x4C
VK_M			0x4D
VK_N			0x4E
VK_O			0x4F
VK_P			0x50
VK_Q			0x51
VK_R			0x52
VK_S			0x53
VK_T			0x54
VK_U			0x55
VK_V			0x56
VK_W			0x57
VK_X			0x58
VK_Y			0x59
VK_Z			0x5A
VK_LWIN           0x5B
VK_RWIN           0x5C
VK_APPS           0x5D
VK_SLEEP          0x5F
VK_NUMPAD0        0x60
VK_NUMPAD1        0x61
VK_NUMPAD2        0x62
VK_NUMPAD3        0x63
VK_NUMPAD4        0x64
VK_NUMPAD5        0x65
VK_NUMPAD6        0x66
VK_NUMPAD7        0x67
VK_NUMPAD8        0x68
VK_NUMPAD9        0x69
VK_MULTIPLY       0x6A
VK_ADD            0x6B
VK_SEPARATOR      0x6C
VK_SUBTRACT       0x6D
VK_DECIMAL        0x6E
VK_DIVIDE         0x6F
VK_F1             0x70
VK_F2             0x71
VK_F3             0x72
VK_F4             0x73
VK_F5             0x74
VK_F6             0x75
VK_F7             0x76
VK_F8             0x77
VK_F9             0x78
VK_F10            0x79
VK_F11            0x7A
VK_F12            0x7B
VK_F13            0x7C
VK_F14            0x7D
VK_F15            0x7E
VK_F16            0x7F
VK_F17            0x80
VK_F18            0x81
VK_F19            0x82
VK_F20            0x83
VK_F21            0x84
VK_F22            0x85
VK_F23            0x86
VK_F24            0x87
VK_NUMLOCK        0x90
VK_SCROLL         0x91
VK_LSHIFT         0xA0
VK_RSHIFT         0xA1
VK_LCONTROL       0xA2
VK_RCONTROL       0xA3
VK_LMENU          0xA4
VK_RMENU          0xA5