/**
 * Hercules DJ Console RMX controller script v1.8.1
 * Copyright (C) 2010  Anders Gunnarsson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

//TODO: Cleanup, create objects from init. 
//Remove led timers when alsa midi is working properly.
HerculesRMX = new function() {
   this.group = "[Master]";
   this.shiftMode = false;
   this.scratchMode = false;
   this.jogPlaylistScrollMode = false;
   this.Controls = [];
   this.Buttons = [];
};

HerculesRMX.addButton = function(buttonName, button, eventHandler) {
   if(eventHandler) {
      var executionEnvironment = this;
      function handler(value) {
         button.state = value;
         executionEnvironment[eventHandler](value);
      }
      button.handler = handler;
   }
   this.Buttons[buttonName] = button; 
};

HerculesRMX.setControlValue = function(control, value) {
   this.Controls[control].setValue(this.group, value);
};


HerculesRMX.ButtonState = {"released":0x00, "pressed":0x7F};
HerculesRMX.LedState =  {"off": 0x00, "on": 0x7F, "blink": 0xFF};
HerculesRMX.Button = function (controlId, blinkId) {
	this.controlId = controlId;
	this.blinkId = blinkId;
	this.state = HerculesRMX.ButtonState.released;
};
HerculesRMX.Button.prototype.setLed = function(lightState) {
	if(lightState == HerculesRMX.LedState.on) {
		midi.sendShortMsg(0xB0,this.controlId,HerculesRMX.LedState.on);
		engine.beginTimer(20, "midi.sendShortMsg(0xB0," + (this.controlId + 0x30) + ", " + HerculesRMX.LedState.off + ")", true);
	} else if(lightState == HerculesRMX.LedState.blink) {
		midi.sendShortMsg(0xB0,this.controlId,HerculesRMX.LedState.on);
		engine.beginTimer(20, "midi.sendShortMsg(0xB0," + (this.controlId + 0x30) + ", " + HerculesRMX.LedState.on + ")", true);
	} else {
		midi.sendShortMsg(0xB0,this.controlId,HerculesRMX.LedState.off);
		engine.beginTimer(20, "midi.sendShortMsg(0xB0," + (this.controlId + 0x30) + ", " + HerculesRMX.LedState.off + ")", true);
	}
};

HerculesRMX.Button.prototype.handleEvent = function(value) {
   this.handler(value);
};

HerculesRMX.Control = function(mappedFunction, softMode) {
	this.minInput = 0;
	this.midInput = 0x3F;
	this.maxInput = 0x7F;
	this.minOutput = -1.0;
	this.midOutput = 0.0;
	this.maxOutput = 1.0;
	this.mappedFunction = mappedFunction;
	this.softMode = softMode;
	this.maxJump = 10;
};

HerculesRMX.Control.prototype.setValue = function(group, inputValue){
	var outputValue = 0;
	if(inputValue <= this.midInput){
		outputValue = this.minOutput + ((inputValue - this.minInput) / (this.midInput - this.minInput)) * (this.midOutput - this.minOutput);
	} else {
		outputValue = this.midOutput + ((inputValue - this.midInput) / (this.maxInput - this.midInput)) * (this.maxOutput - this.midOutput);
	}
	if(this.softMode){ 
		var currentValue = engine.getValue(group, this.mappedFunction);
		var currentRelative = 0.0;
		if(currentValue <= this.midOutput){
			currentRelative = this.minInput + ((currentValue - this.minOutput) / (this.midOutput - this.minOutput)) * (this.midInput - this.minInput);
		} else {
			currentRelative = this.midInput + ((currentValue - this.midOutput) / (this.maxOutput - this.midOutput)) * (this.maxInput - this.midInput);
		}
		if(inputValue > currentRelative - this.maxJump && inputValue < currentRelative + this.maxJump) {
			engine.setValue(group, this.mappedFunction, outputValue);
		}
	} else {
		engine.setValue(group, this.mappedFunction, outputValue);
	}
};

HerculesRMX.shiftHandler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      this.shiftMode = true;
      if(HerculesRMX.scratchMode) {
         HerculesRMX.scratchMode = false;
         HerculesRMX.Buttons.Scratch.setLed(HerculesRMX.LedState.off);
      } else {
         HerculesRMX.scratchMode = true;
         HerculesRMX.Buttons.Scratch.setLed(HerculesRMX.LedState.on);
      }
   } else {
      this.shiftMode = false;
   }
};

HerculesRMX.upHandler = function(value) {
   this.jogPlaylistScrollMode = this.Buttons.Up.state + this.Buttons.Down.state > 0;
   if(value == HerculesRMX.ButtonState.pressed) {
      engine.setValue("[Playlist]", "SelectPrevTrack", 1);
   }
};

HerculesRMX.downHandler = function(value) {
   this.jogPlaylistScrollMode = this.Buttons.Up.state + this.Buttons.Down.state > 0;
   if(value == HerculesRMX.ButtonState.pressed) {
      engine.setValue("[Playlist]", "SelectNextTrack", 1);
   }
};

HerculesRMX.leftHandler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      engine.setValue("[Playlist]", "SelectPrevPlaylist", 1);
   }
};

HerculesRMX.rightHandler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      engine.setValue("[Playlist]", "SelectNextPlaylist", 1);
   }
};

HerculesRMX.addButton("Scratch", new HerculesRMX.Button(0x29), "shiftHandler");
HerculesRMX.addButton("Up", new HerculesRMX.Button(0x2A), "upHandler");
HerculesRMX.addButton("Down", new HerculesRMX.Button(0x2B), "downHandler");
HerculesRMX.addButton("Left", new HerculesRMX.Button(0x2C), "leftHandler");
HerculesRMX.addButton("Right", new HerculesRMX.Button(0x2D), "rightHandler");

HerculesRMX.Controls = {
      "Balance" : new HerculesRMX.Control("balance", false),
      "Volume" : new HerculesRMX.Control("volume", false),
      "CrossFader" : new HerculesRMX.Control("crossfader", false),
      "HeadPhoneMix" : new HerculesRMX.Control("headMix", false),
      "FlangerDepth" : new HerculesRMX.Control("lfoDepth", false),
      "FlangerDelay" : new HerculesRMX.Control("lfoDelay", false),
      "FlangerPeriod" : new HerculesRMX.Control("lfoPeriod", false)
};
HerculesRMX.Controls.Volume.minOutput = 0.0;
HerculesRMX.Controls.Volume.midOutput = 1.0;
HerculesRMX.Controls.Volume.maxOutput = 5.0;
HerculesRMX.Controls.FlangerDepth.minOutput = 0.0;
HerculesRMX.Controls.FlangerDepth.midOutput = 0.5;
HerculesRMX.Controls.FlangerDepth.maxOutput = 1.0;
HerculesRMX.Controls.FlangerDelay.minOutput = 50;
HerculesRMX.Controls.FlangerDelay.midOutput = 5000;
HerculesRMX.Controls.FlangerDelay.maxOutput = 10000;
HerculesRMX.Controls.FlangerPeriod.minOutput = 50000;
HerculesRMX.Controls.FlangerPeriod.midOutput = 1000000;
HerculesRMX.Controls.FlangerPeriod.maxOutput = 2000000;

HerculesRMX.balanceHandler = function(value) {
   this.Controls.Balance.setValue(this.group, value);
};

HerculesRMX.volumeHandler = function(value) {
   this.Controls.Volume.setValue(this.group, value);
};

HerculesRMX.crossFaderHandler = function(value) {
   this.Controls.CrossFader.setValue(this.group, value);
};

HerculesRMX.headPhoneMixHandler = function(value) {
   this.Controls.HeadPhoneMix.setValue(this.group, value);
};

/**
 * Deck
 * @param deckNumber
 * @param group
 */
HerculesRMX.Deck = function (deckNumber, group) {
	this.deckNumber = deckNumber;
	this.group = group;
	this.shiftMode = false;
	this.scratching = false;
	this.scratchTimer = -1;
	this.cuePlaying = false;
	this.playing = false;
	this.Buttons = [];
};

HerculesRMX.Deck.prototype.jogMove = function(jogValue) {
   if(HerculesRMX.jogPlaylistScrollMode) {
      if (jogValue > 0) {
         engine.setValue("[Playlist]","SelectNextTrack", 1);
      } else if (jogValue < 0) {
         engine.setValue("[Playlist]","SelectPrevTrack", 1);
      }
   } else if(HerculesRMX.scratchMode) {
		if(!this.scratching) {
			this.scratching = true;
			engine.scratchEnable(this.deckNumber, 128, 45, 1.0/8, (1.0/8)/32);
		} else {
			engine.stopTimer(this.scratchTimer);
		}
		engine.scratchTick(this.deckNumber, jogValue);
		this.scratchTimer = engine.beginTimer(20, "HerculesRMX.GetDeck('" + this.group + "').stopScratching()", true);
	} else {
		engine.setValue(this.group,"jog", jogValue);
	}
};

HerculesRMX.Deck.prototype.stopScratching = function() {
	this.scratching = false;
	engine.scratchDisable(this.deckNumber);
	this.scratchTimer  = -1;
};

HerculesRMX.Deck.prototype.setControlValue = HerculesRMX.setControlValue;

HerculesRMX.Deck.prototype.pitchResetHandler = function(value) {
	if(value == HerculesRMX.ButtonState.pressed) {
		engine.setValue(this.group,"rate",0);
	    this.Buttons.PitchReset.setLed(HerculesRMX.LedState.on);
	}
};

HerculesRMX.Deck.prototype.syncHandler = function(value) {
	if(value == HerculesRMX.ButtonState.pressed) {
      engine.setValue(this.group,"beatsync",0);
      this.Buttons.Sync.setLed(HerculesRMX.LedState.on);
	}
};

HerculesRMX.Deck.prototype.keypad1Handler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      if(HerculesRMX.shiftMode) {
         if(engine.getValue(this.group,"flanger") == 0) {
            engine.setValue(this.group,"flanger",1);
         } else {
            engine.setValue(this.group,"flanger",0);
         }
      } else if(this.shiftMode) {
         engine.setValue(this.group,"hotcue_1_set", 1);
      } else {
         engine.setValue(this.group,"hotcue_1_goto", 1);
      }
   }
};

HerculesRMX.Deck.prototype.keypad2Handler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      if(this.shiftMode) {
         engine.setValue(this.group,"hotcue_2_set", 1);
      } else {
         engine.setValue(this.group,"hotcue_2_goto", 1);
      }
   }
};

HerculesRMX.Deck.prototype.keypad3Handler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      if(this.shiftMode) {
         engine.setValue(this.group,"hotcue_3_set", 1);
      } else {
         engine.setValue(this.group,"hotcue_3_goto", 1);
      }
   }
};

HerculesRMX.Deck.prototype.keypad4Handler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      if(HerculesRMX.shiftMode) {
         engine.setValue(this.group,"reverse",1);
      } else if(this.shiftMode) {
         engine.setValue(this.group,"loop_in",1);
      }
   } else {
      engine.setValue(this.group,"reverse",0);
   }
};

HerculesRMX.Deck.prototype.keypad5Handler = function(value) {
   if(this.shiftMode) {
      if(value == HerculesRMX.ButtonState.pressed) {
         engine.setValue(this.group,"loop_out",1);
      }
   }
};

HerculesRMX.Deck.prototype.keypad6Handler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      if(this.shiftMode) {
         var loopIn = engine.getValue(this.group, "loop_start_position");
         var loopOut = engine.getValue(this.group, "loop_end_position");
         var loopLength = loopOut - loopIn;
         if (loopIn != -1 && loopOut != -1 && loopLength >= 2) {
            engine.setValue(this.group,"loop_end_position",loopIn + loopLength / 2);
         }
      } else {
         engine.setValue(this.group,"reloop_exit",1);
      }
   }
};

HerculesRMX.Deck.prototype.previousHandler = function(value) {
   if(this.Buttons.Keypad1.state == HerculesRMX.ButtonState.pressed) {
    //Move hotcue 1 backwards
      var hotcue = engine.getValue(this.group, "hotcue_1_position");
      var newPosition = hotcue - 400;
      if(newPosition > 0) {
         engine.setValue(this.group, "hotcue_1_position", newPosition);
      }
   } else if(this.Buttons.Keypad2.state == HerculesRMX.ButtonState.pressed) {
    //Move hotcue 2 backwards
      var hotcue = engine.getValue(this.group, "hotcue_2_position");
      var newPosition = hotcue - 400;
      if(newPosition > 0) {
         engine.setValue(this.group, "hotcue_2_position", newPosition);
      }
   } else if(this.Buttons.Keypad3.state == HerculesRMX.ButtonState.pressed) {
      //Move hotcue 3 backwards
      var hotcue = engine.getValue(this.group, "hotcue_3_position");
      var newPosition = hotcue - 400;
      if(newPosition > 0) {
         engine.setValue(this.group, "hotcue_3_position", newPosition);
      }
   } else if(this.Buttons.Keypad4.state == HerculesRMX.ButtonState.pressed) {
      //Move loop-in backwards
      var loopIn = engine.getValue(this.group, "loop_start_position");
      var newPosition = loopIn - 400;
      if(newPosition > 0) {
         engine.setValue(this.group, "loop_start_position", newPosition);
      }
   } else if(this.Buttons.Keypad5.state == HerculesRMX.ButtonState.pressed) {
      //Move loop-out backwards
      var loopIn = engine.getValue(this.group, "loop_start_position");
      var loopOut = engine.getValue(this.group, "loop_end_position");
      var newPosition = loopOut - 400;
      if(newPosition > loopIn) {
         engine.setValue(this.group, "loop_end_position", newPosition);
      }
   } else {
      engine.setValue(this.group,"back",value);
   }
};

HerculesRMX.Deck.prototype.nextHandler = function(value) {
   //TODO: Fix movement of hotcues & loops out of track bounds
   if(this.Buttons.Keypad1.state == HerculesRMX.ButtonState.pressed) {
      //Move hotcue 1 forwards
        var hotcue = engine.getValue(this.group, "hotcue_1_position");
        var newPosition = hotcue + 400;
        if(hotcue != -1) {
           engine.setValue(this.group, "hotcue_1_position", newPosition);
        }
     } else if(this.Buttons.Keypad2.state == HerculesRMX.ButtonState.pressed) {
      //Move hotcue 2 forwards
        var hotcue = engine.getValue(this.group, "hotcue_2_position");
        var newPosition = hotcue + 400;
        if(hotcue != -1) {
           engine.setValue(this.group, "hotcue_2_position", newPosition);
        }
     } else if(this.Buttons.Keypad3.state == HerculesRMX.ButtonState.pressed) {
        //Move hotcue 3 forwards
        var hotcue = engine.getValue(this.group, "hotcue_3_position");
        var newPosition = hotcue + 400;
        if(hotcue != -1 > 0) {
           engine.setValue(this.group, "hotcue_3_position", newPosition);
        }
     } else if(this.Buttons.Keypad4.state == HerculesRMX.ButtonState.pressed) {
      //Move loop-in forwards
      var loopIn = engine.getValue(this.group, "loop_start_position");
      var loopOut = engine.getValue(this.group, "loop_end_position");
      var newPosition = loopIn + 400;
      if(newPosition < loopOut) {
         engine.setValue(this.group, "loop_start_position", newPosition);
      }
   } else if(this.Buttons.Keypad5.state == HerculesRMX.ButtonState.pressed) {
      //Move loop-out forwards
      var loopOut = engine.getValue(this.group, "loop_end_position");
      var newPosition = loopOut + 400;
      engine.setValue(this.group, "loop_end_position", newPosition);
   } else {
      engine.setValue(this.group,"fwd",value);
   }
};

HerculesRMX.Deck.prototype.playPauseHandler = function(value) {
};

HerculesRMX.Deck.prototype.cueHandler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      //var position = engine.getValue(this.group,"playposition") * engine.getValue(this.group, "duration") * engine.getValue(this.group, "duration");
      //var atCuePoint = engine.getValue(this.group,"cue_point") == position;
      //print("at cue: " + atCuePoint + ", playposition: " + position);
      engine.setValue(this.group,"cue_default",1);
      this.playing = false;
      this.Buttons.PlayPause.setLed(HerculesRMX.LedState.off);
      //if(atCuePoint) {
         this.cuePlaying = true;
         this.Buttons.Cue.setLed(HerculesRMX.LedState.on);
      //}
   } else {
      engine.setValue(this.group,"cue_default",0);
      if(this.cuePlaying) {
         this.cuePlaying = false;
      }
      this.Buttons.Cue.setLed(HerculesRMX.LedState.off);
   }
};

HerculesRMX.Deck.prototype.killHighHandler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      var filterStatus = engine.getValue(this.group, "filterHighKill");
      if(filterStatus) {
         engine.setValue(this.group, "filterHighKill", 0);
      } else {
         engine.setValue(this.group, "filterHighKill", 1);
      }
   }
};

HerculesRMX.Deck.prototype.killMidHandler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      var filterStatus = engine.getValue(this.group, "filterMidKill");
      if(filterStatus) {
         engine.setValue(this.group, "filterMidKill", 0);
      } else {
         engine.setValue(this.group, "filterMidKill", 1);
      }
   }
};

HerculesRMX.Deck.prototype.killLowHandler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      var filterStatus = engine.getValue(this.group, "filterLowKill");
      if(filterStatus) {
         engine.setValue(this.group, "filterLowKill", 0);
      } else {
         engine.setValue(this.group, "filterLowKill", 1);
      }
   }
};

HerculesRMX.Deck.prototype.loadHandler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      engine.setValue(this.group, "LoadSelectedTrack", 1);
   }
};

HerculesRMX.Deck.prototype.cueSelectHandler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      var filterStatus = engine.getValue(this.group, "pfl");
      if(filterStatus) {
         engine.setValue(this.group, "pfl", 0);
         this.Buttons.CueSelect.setLed(HerculesRMX.LedState.on);
      } else {
         engine.setValue(this.group, "pfl", 1);
         this.Buttons.CueSelect.setLed(HerculesRMX.LedState.off);
      }
   }
};

HerculesRMX.Deck.prototype.gainHandler = function(value) {
   this.Controls.Gain.setValue(this.group, value);
};

HerculesRMX.Deck.prototype.trebleHandler = function(value) {
   if(HerculesRMX.shiftMode) {
      //Flanger
      HerculesRMX.Controls.FlangerDepth.setValue("[Flanger]", value);
   } else if(this.shiftMode) {
      //Soft set
      this.Controls.Treble.softMode = true;
      this.Controls.Treble.setValue(this.group, value);
      this.Controls.Treble.softMode = false;
   } else {
      this.Controls.Treble.setValue(this.group, value);
   }
};

HerculesRMX.Deck.prototype.mediumHandler = function(value) {
   if(HerculesRMX.shiftMode) {
      //Flanger
      HerculesRMX.Controls.FlangerDelay.setValue("[Flanger]", value);
   } else if(this.shiftMode) {
      //Soft set
      this.Controls.Medium.softMode = true;
      this.Controls.Medium.setValue(this.group, value);
      this.Controls.Medium.softMode = false;
   } else {
      this.Controls.Medium.setValue(this.group, value);
   }
};

HerculesRMX.Deck.prototype.bassHandler = function(value) {
   if(HerculesRMX.shiftMode) {
      //Flanger
      HerculesRMX.Controls.FlangerPeriod.setValue("[Flanger]", value);
   } else if(this.shiftMode) {
      //Soft set
      this.Controls.Bass.softMode = true;
      this.Controls.Bass.setValue(this.group, value);
      this.Controls.Bass.softMode = false;
   } else {
      this.Controls.Bass.setValue(this.group, value);
   }
};

HerculesRMX.Deck.prototype.volHandler = function(value) {
   this.Controls.Vol.setValue(this.group, value);
};

HerculesRMX.Deck.prototype.pitchHandler = function(value) {
   this.Controls.Pitch.setValue(this.group, value);
};

HerculesRMX.Deck.prototype.shiftHandler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      this.shiftMode = true;
   } else {
      this.shiftMode = false;
   }
};

HerculesRMX.Deck.prototype.stopHandler = function(value) {
   if(value == HerculesRMX.ButtonState.pressed) {
      engine.setValue(this.group, "cue_default", 0);
      engine.setValue(this.group, "play", 0);
      engine.setValue(this.group, "start", 0);
   }
};

HerculesRMX.Deck.prototype.addButton = HerculesRMX.addButton;

HerculesRMX.Decks = {"Left":new HerculesRMX.Deck(1,"[Channel1]"), "Right":new HerculesRMX.Deck(2,"[Channel2]")};
HerculesRMX.GroupToDeck = {"[Channel1]":"Left", "[Channel2]":"Right"};

HerculesRMX.GetDeck = function(group) {
   try {
      return HerculesRMX.Decks[HerculesRMX.GroupToDeck[group]];
   } catch(ex) {
      return null;
   }
};

HerculesRMX.Decks.Left.addButton("Keypad1", new HerculesRMX.Button(0x01), "keypad1Handler");
HerculesRMX.Decks.Left.addButton("Keypad2", new HerculesRMX.Button(0x02), "keypad2Handler");
HerculesRMX.Decks.Left.addButton("Keypad3", new HerculesRMX.Button(0x03), "keypad3Handler");
HerculesRMX.Decks.Left.addButton("Keypad4", new HerculesRMX.Button(0x04), "keypad4Handler");
HerculesRMX.Decks.Left.addButton("Keypad5", new HerculesRMX.Button(0x05), "keypad5Handler");
HerculesRMX.Decks.Left.addButton("Keypad6", new HerculesRMX.Button(0x06), "keypad6Handler");
HerculesRMX.Decks.Left.addButton("Sync", new HerculesRMX.Button(0x07, 0x37), "syncHandler");
HerculesRMX.Decks.Left.addButton("BeatLock", new HerculesRMX.Button(0x08, 0x38), null);
HerculesRMX.Decks.Left.addButton("Previous", new HerculesRMX.Button(0x09), "previousHandler");
HerculesRMX.Decks.Left.addButton("Next", new HerculesRMX.Button(0x0A), "nextHandler");
HerculesRMX.Decks.Left.addButton("PlayPause", new HerculesRMX.Button(0x0B, 0x3B), "playPauseHandler");
HerculesRMX.Decks.Left.addButton("Cue", new HerculesRMX.Button(0x0C, 0x3C), "cueHandler");
HerculesRMX.Decks.Left.addButton("Shift",  new HerculesRMX.Button(0x0D), "shiftHandler");
HerculesRMX.Decks.Left.addButton("KillHigh", new HerculesRMX.Button(0x0E), "killHighHandler");
HerculesRMX.Decks.Left.addButton("KillMid", new HerculesRMX.Button(0x0F), "killMidHandler");
HerculesRMX.Decks.Left.addButton("KillLow", new HerculesRMX.Button(0x10), "killLowHandler");
HerculesRMX.Decks.Left.addButton("PitchReset", new HerculesRMX.Button(0x11, 0x41), "pitchResetHandler");
HerculesRMX.Decks.Left.addButton("Load", new HerculesRMX.Button(0x12), "loadHandler");
HerculesRMX.Decks.Left.addButton("Source", new HerculesRMX.Button(0x13, 0x43), null);
HerculesRMX.Decks.Left.addButton("CueSelect", new HerculesRMX.Button(0x14, 0x44), "cueSelectHandler");
HerculesRMX.Decks.Left.addButton("Stop",  new HerculesRMX.Button(0x0D), "stopHandler");

HerculesRMX.Decks.Left.Controls = {
      "Gain" : new HerculesRMX.Control("pregain", false),
      "Treble" : new HerculesRMX.Control("filterHigh", false),
      "Medium" : new HerculesRMX.Control("filterMid", false),
      "Bass" : new HerculesRMX.Control("filterLow", false),
      "Vol" : new HerculesRMX.Control("volume", false),
      "Pitch" : new HerculesRMX.Control("rate", false)
};
HerculesRMX.Decks.Left.Controls.Gain.minOutput = 0.0;
HerculesRMX.Decks.Left.Controls.Gain.midOutput = 1.0;
HerculesRMX.Decks.Left.Controls.Gain.maxOutput = 4.0;
HerculesRMX.Decks.Left.Controls.Treble.minOutput = 0.0;
HerculesRMX.Decks.Left.Controls.Treble.midOutput = 1.0;
HerculesRMX.Decks.Left.Controls.Treble.maxOutput = 4.0;
HerculesRMX.Decks.Left.Controls.Medium.minOutput = 0.0;
HerculesRMX.Decks.Left.Controls.Medium.midOutput = 1.0;
HerculesRMX.Decks.Left.Controls.Medium.maxOutput = 4.0;
HerculesRMX.Decks.Left.Controls.Bass.minOutput = 0.0;
HerculesRMX.Decks.Left.Controls.Bass.midOutput = 1.0;
HerculesRMX.Decks.Left.Controls.Bass.maxOutput = 4.0;
HerculesRMX.Decks.Left.Controls.Vol.minOutput = 0.0;
HerculesRMX.Decks.Left.Controls.Vol.midOutput = 0.4;
HerculesRMX.Decks.Left.Controls.Vol.maxOutput = 1.0;
HerculesRMX.Decks.Left.Controls.Pitch.midInput = 0x40;

HerculesRMX.Decks.Right.addButton("Keypad1",  new HerculesRMX.Button(0x19), "keypad1Handler");
HerculesRMX.Decks.Right.addButton("Keypad2", new HerculesRMX.Button(0x1A), "keypad2Handler");
HerculesRMX.Decks.Right.addButton("Keypad3", new HerculesRMX.Button(0x1B), "keypad3Handler");
HerculesRMX.Decks.Right.addButton("Keypad4", new HerculesRMX.Button(0x1C), "keypad4Handler");
HerculesRMX.Decks.Right.addButton("Keypad5", new HerculesRMX.Button(0x1D), "keypad5Handler");
HerculesRMX.Decks.Right.addButton("Keypad6", new HerculesRMX.Button(0x1E), "keypad6Handler");
HerculesRMX.Decks.Right.addButton("Sync", new HerculesRMX.Button(0x1F, 0x5F), "syncHandler");
HerculesRMX.Decks.Right.addButton("BeatLock", new HerculesRMX.Button(0x15, 0x60), null);
HerculesRMX.Decks.Right.addButton("Previous", new HerculesRMX.Button(0x21), "previousHandler");
HerculesRMX.Decks.Right.addButton("Next", new HerculesRMX.Button(0x22), "nextHandler");
HerculesRMX.Decks.Right.addButton("PlayPause", new HerculesRMX.Button(0x23, 0x53), "playPauseHandler");
HerculesRMX.Decks.Right.addButton("Cue", new HerculesRMX.Button(0x24, 0x54), "cueHandler");
HerculesRMX.Decks.Right.addButton("Shift", new HerculesRMX.Button(0x25), "shiftHandler");
HerculesRMX.Decks.Right.addButton("KillHigh", new HerculesRMX.Button(0x26), "killHighHandler");
HerculesRMX.Decks.Right.addButton("KillMid", new HerculesRMX.Button(0x27), "killMidHandler");
HerculesRMX.Decks.Right.addButton("KillLow", new HerculesRMX.Button(0x28), "killLowHandler");
HerculesRMX.Decks.Right.addButton("PitchReset", new HerculesRMX.Button(0x20, 0x55), "pitchResetHandler");
HerculesRMX.Decks.Right.addButton("Load", new HerculesRMX.Button(0x16), "loadHandler");
HerculesRMX.Decks.Right.addButton("Source", new HerculesRMX.Button(0x17, 0x57), null);
HerculesRMX.Decks.Right.addButton("CueSelect", new HerculesRMX.Button(0x18, 0x58), "cueSelectHandler");
HerculesRMX.Decks.Right.addButton("Stop", new HerculesRMX.Button(0x25), "stopHandler");

HerculesRMX.Decks.Right.Controls = {
      "Gain" : new HerculesRMX.Control("pregain", false),
      "Treble" : new HerculesRMX.Control("filterHigh", false),
      "Medium" : new HerculesRMX.Control("filterMid", false),
      "Bass" : new HerculesRMX.Control("filterLow", false),
      "Vol" : new HerculesRMX.Control("volume", false),
      "Pitch" : new HerculesRMX.Control("rate", false)
};
HerculesRMX.Decks.Right.Controls.Gain.minOutput = 0.0;
HerculesRMX.Decks.Right.Controls.Gain.midOutput = 1.0;
HerculesRMX.Decks.Right.Controls.Gain.maxOutput = 4.0;
HerculesRMX.Decks.Right.Controls.Treble.minOutput = 0.0;
HerculesRMX.Decks.Right.Controls.Treble.midOutput = 1.0;
HerculesRMX.Decks.Right.Controls.Treble.maxOutput = 4.0;
HerculesRMX.Decks.Right.Controls.Medium.minOutput = 0.0;
HerculesRMX.Decks.Right.Controls.Medium.midOutput = 1.0;
HerculesRMX.Decks.Right.Controls.Medium.maxOutput = 4.0;
HerculesRMX.Decks.Right.Controls.Bass.minOutput = 0.0;
HerculesRMX.Decks.Right.Controls.Bass.midOutput = 1.0;
HerculesRMX.Decks.Right.Controls.Bass.maxOutput = 4.0;
HerculesRMX.Decks.Right.Controls.Vol.minOutput = 0.0;
HerculesRMX.Decks.Right.Controls.Vol.midOutput = 0.4;
HerculesRMX.Decks.Right.Controls.Vol.maxOutput = 1.0;
HerculesRMX.Decks.Right.Controls.Pitch.midInput = 0x40;


//Mapping functions
HerculesRMX.volume = function(channel, control, value, status, group) {
   HerculesRMX.volumeHandler(value);
};

HerculesRMX.balance = function(channel, control, value, status, group) {
   HerculesRMX.balanceHandler(value);
};

HerculesRMX.crossFader = function(channel, control, value, status, group) {
   HerculesRMX.crossFaderHandler(value);
};

HerculesRMX.headPhoneMix = function(channel, control, value, status, group) {
   HerculesRMX.headPhoneMixHandler(value);
};

HerculesRMX.up = function (channel, control, value, status, group) {
   HerculesRMX.Buttons.Up.handleEvent(value);
};

HerculesRMX.down = function (channel, control, value, status, group) {
   HerculesRMX.Buttons.Down.handleEvent(value);
};

HerculesRMX.left = function (channel, control, value, status, group) {
   HerculesRMX.Buttons.Left.handleEvent(value);
};

HerculesRMX.right = function (channel, control, value, status, group) {
   HerculesRMX.Buttons.Right.handleEvent(value);
};

HerculesRMX.scratch = function (channel, control, value, status, group) {
   HerculesRMX.Buttons.Scratch.handleEvent(value);
};

HerculesRMX.gain = function(channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.gainHandler(value);
};

HerculesRMX.rate = function(channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.pitchHandler(value);
};

HerculesRMX.treble = function(channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.trebleHandler(value);
};

HerculesRMX.medium = function(channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.mediumHandler(value);
};

HerculesRMX.bass = function(channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.bassHandler(value);
};

HerculesRMX.deckVolume = function(channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.volHandler(value);
};

HerculesRMX.jog_wheel = function (channel, control, value, status, group) {
// 7F > 40: CCW Slow > Fast - 127 > 64 
// 01 > 3F: CW Slow > Fast - 0 > 63
  var jogValue = value >=0x40 ? value - 0x80 : value; // -64 to +63, - = CCW, + = CW
  HerculesRMX.GetDeck(group).jogMove(jogValue);
};

HerculesRMX.cue = function (channel, control, value, status, group) {
  var deck = HerculesRMX.GetDeck(group);
  deck.Buttons.Cue.handleEvent(value);
};

HerculesRMX.beatSync = function (channel, control, value, status, group) {
  var deck = HerculesRMX.GetDeck(group);
  deck.Buttons.Sync.handleEvent(value);
};

HerculesRMX.rateReset = function (channel, control, value, status, group) {
  var deck = HerculesRMX.GetDeck(group);
  deck.Buttons.PitchReset.handleEvent(value);
};

HerculesRMX.play = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.PlayPause.handleEvent(value);
};

HerculesRMX.stop = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.Stop.handleEvent(value);
};

HerculesRMX.shift = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.Shift.handleEvent(value);
};

HerculesRMX.keypad1 = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.Keypad1.handleEvent(value);
};
 
HerculesRMX.keypad2 = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.Keypad2.handleEvent(value);
};

HerculesRMX.keypad3 = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.Keypad3.handleEvent(value);
};
 
HerculesRMX.keypad4 = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.Keypad4.handleEvent(value);
};

HerculesRMX.keypad5 = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.Keypad5.handleEvent(value);
};
 
HerculesRMX.keypad6 = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.Keypad6.handleEvent(value);
};

HerculesRMX.next = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.Next.handleEvent(value);
};

HerculesRMX.previous = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.Previous.handleEvent(value);
};

HerculesRMX.load = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.Load.handleEvent(value);
};

HerculesRMX.cueSelect = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.CueSelect.handleEvent(value);
};

HerculesRMX.killLow = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.KillLow.handleEvent(value);
};


HerculesRMX.killMid = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.KillMid.handleEvent(value);
};


HerculesRMX.killHigh = function (channel, control, value, status, group) {
   var deck = HerculesRMX.GetDeck(group);
   deck.Buttons.KillHigh.handleEvent(value);
};



HerculesRMX.init = function (id) {    // called when the MIDI device is opened & set up
   HerculesRMX.killLeds();
   
   engine.connectControl("[Channel1]","rate","HerculesRMX.rateChange");
   engine.connectControl("[Channel2]","rate","HerculesRMX.rateChange");
    
   print ("HerculesRMX id: \""+id+"\" initialized.");
};

HerculesRMX.shutdown = function() {
   HerculesRMX.killLeds();
   
   print ("HerculesRMX shutdown.");
};

HerculesRMX.killLeds = function() {
   HerculesRMX.Buttons.Scratch.setLed(HerculesRMX.LedState.off);
   //TODO: remove timers when alsa midi work properly.
   var button;
   var time = 20;
   for (var key in HerculesRMX.Decks.Left.Buttons) {
      engine.beginTimer(time, "HerculesRMX.Decks.Left.Buttons['" + key + "'].setLed(HerculesRMX.LedState.off)", true);
      time = time + 5;
   }
   for (var key in HerculesRMX.Decks.Right.Buttons) {
      engine.beginTimer(time, "HerculesRMX.Decks.Right.Buttons['" + key + "'].setLed(HerculesRMX.LedState.off)", true);
      time = time + 5;
   }
}

//Rate change event handler to reset sync and reset leds
//TODO: Need some way to check sync state here.
HerculesRMX.rateChange = function (value, group) {
   if (HerculesRMX.Decks.Left.Buttons.Sync.state != HerculesRMX.ButtonState.pressed) {
      HerculesRMX.Decks.Left.Buttons.Sync.setLed(HerculesRMX.LedState.off);
   }
   if (HerculesRMX.Decks.Right.Buttons.Sync.state != HerculesRMX.ButtonState.pressed) {
      engine.beginTimer(25, "HerculesRMX.Decks.Right.Buttons.Sync.setLed(HerculesRMX.LedState.off)", true);
   }
   if (value != 0.0) {
      var deck = HerculesRMX.GetDeck(group);
      engine.beginTimer(30, "HerculesRMX.GetDeck('" + group + "').Buttons.PitchReset.setLed(HerculesRMX.LedState.off)", true);
   }
};

//TODO: Keep stop functionality?
//HerculesRMX.stop_and_reset_track = function (channel, control, value, status, group) {
//   if (engine.getValue(group, "duration") == 0) { if (value) print("No song on " + group); return; };
//   if (value > 0) {
//        engine.setValue(group,"cue_default",0);
//   engine.setValue(group,"play",0);
//   engine.setValue(group,"start",0);
//
//        HerculesRMX.cueButton[group] = false;
//        HerculesRMX.cuePlay[group] = false;
//        midi.sendShortMsg(0xB0, HerculesRMX.leds[group + " cue"], HerculesRMX.ledOff);
//   }
//};
//
//HerculesRMX.vinyl_stop_and_return = function (channel, control, value, status, group) {
//   if (engine.getValue(group, "duration") == 0) { if (value) print("No song on " + group); return; };
//
//   if (value) {
//      if (engine.getValue(group,"play")) {
//        HerculesRMX.cuePlay[group] = true;
//        midi.sendShortMsg(0xB0, HerculesRMX.leds[group + " cue"], HerculesRMX.ledOff);     
//        var playposition = engine.getValue(group,"playposition");
//        engine.setValue(group,"play",0);
//   HerculesRMX.stopJog[group] = 1.0;
//   HerculesRMX.decayLast = new Date().getTime();
//   engine.setValue(group,"scratch",HerculesRMX.stopJog[group]);
//   HerculesRMX.stoping = true;
//   return;
//      } else {
//        engine.setValue(group,"playposition",0);
//      }
//   }
//};


