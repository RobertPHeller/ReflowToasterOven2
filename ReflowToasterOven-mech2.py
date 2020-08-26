#!/usr/local/bin/FreeCAD018
#*****************************************************************************
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Robert Heller
#  Created       : Mon Aug 24 11:24:56 2020
#  Last Modified : <200826.1322>
#
#  Description	
#
#  Notes
#
#  History
#	
#*****************************************************************************
#
#    Copyright (C) 2020  Robert Heller D/B/A Deepwoods Software
#			51 Locke Hill Road
#			Wendell, MA 01379-9728
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# 
#
#*****************************************************************************


import Part, TechDraw, Spreadsheet, TechDrawGui
import FreeCADGui
from FreeCAD import Console
from FreeCAD import Base
import FreeCAD as App
import os
import sys
sys.path.append(os.path.dirname(__file__))

import datetime

from ReflowToasterOven2Board import ReflowToasterOven2Board
from ReflowToasterOven2Box   import UpperBox, LowerBox
from TFT_Display             import TFT_Display
from LargePiezo              import LargePiezo
from D2425                   import D2425
from ACPower                 import ACPowerEntry, ACReceptacle
from HeatSink                import HeatSink, HeatSinkMain

def MM2Mils(MM):
    return (MM/25.4)*1000
    
class PushButtonSwitchHole(object):
    _radius = 290/2.0
    def __init__(self,origin,direction='Z',height=0):
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        if direction == 'Z':
            self.normal = Base.Vector(0,0,1)
            self.extrude = Base.Vector(0,0,height)
        elif direction == 'X':
            self.normal = Base.Vector(1,0,0)
            self.extrude = Base.Vector(height,0,0)
        elif direction == 'Y':
            self.normal = Base.Vector(0,1,0)
            self.extrude = Base.Vector(0,height,0)
        else:
             raise RuntimeError("Illegal value for direction")
    def Hole(self):
        return Part.Face(Part.Wire(Part.makeCircle(self._radius,self.origin,\
                                   self.normal))).extrude(self.extrude)


class ReflowToasterOvenControlBox(object):
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        self.upperbox = UpperBox(name+"_upperbox",self.origin)
        self.lowerbox = LowerBox(name+"_lowerbox",self.origin)
        upperboxCenterY = self.upperbox.InsideFrontY + (self.upperbox.InsideWidth/2.0)
        MOY = upperboxCenterY - (ReflowToasterOven2Board.BoardWidth()/2.0)
        MOX = self.upperbox.InsideFrontX
        MOZ = self.upperbox.InsideBottomZ + MM2Mils(6.0)
        self.board = ReflowToasterOven2Board(name+"_board",Base.Vector(MOX,MOY,MOZ))
        self.board_standoff1 = self.board.StandOff(1,6.0)
        self.board_standoff2 = self.board.StandOff(2,6.0)
        self.board_standoff3 = self.board.StandOff(3,6.0)
        self.board_standoff4 = self.board.StandOff(4,6.0)
        self.upperbox.cutbox(self.board.mountingHole(1,height=-self.upperbox.WallThickness,standoffMM=-6))
        self.upperbox.cutbox(self.board.mountingHole(2,height=-self.upperbox.WallThickness,standoffMM=-6))
        self.upperbox.cutbox(self.board.mountingHole(3,height=-self.upperbox.WallThickness,standoffMM=-6))
        self.upperbox.cutbox(self.board.mountingHole(4,height=-self.upperbox.WallThickness,standoffMM=-6))
        self.lowerbox.cutbox(self.board.mountingHole(1,height=self.lowerbox.WallThickness,standoffMM=-8))
        self.lowerbox.cutbox(self.board.mountingHole(2,height=self.lowerbox.WallThickness,standoffMM=-8))
        self.lowerbox.cutbox(self.board.mountingHole(3,height=self.lowerbox.WallThickness,standoffMM=-8))
        self.lowerbox.cutbox(self.board.mountingHole(4,height=self.lowerbox.WallThickness,standoffMM=-8))
        boxLidCenterY = self.upperbox.OutsideFrontY + (self.upperbox.OutsideWidth/2.0)
        boxInnerRightX = self.upperbox.InsideFrontX + self.upperbox.InsideLength
        displayX = boxInnerRightX - TFT_Display.BoardLength()
        displayY = boxLidCenterY - (TFT_Display.BoardWidth()/2.0)
        displayZ = self.upperbox.TopZ - 100
        self.display = TFT_Display(name+"_display",Base.Vector(displayX,displayY,displayZ))
        self.upperbox.cutlid(self.display.MountingHole(1,self.upperbox.TopZ,self.upperbox.WallThickness))
        self.upperbox.cutlid(self.display.MountingHole(2,self.upperbox.TopZ,self.upperbox.WallThickness))
        self.upperbox.cutlid(self.display.MountingHole(3,self.upperbox.TopZ,self.upperbox.WallThickness))
        self.upperbox.cutlid(self.display.MountingHole(4,self.upperbox.TopZ,self.upperbox.WallThickness))
        self.upperbox.cutlid(self.display.DisplayCutout(self.upperbox.TopZ,self.upperbox.WallThickness))
        switchesZ = self.upperbox.TopZ
        switchesHeight = self.upperbox.WallThickness
        switchesX = displayX - 500
        switch1Y  = boxLidCenterY + 750
        switch2Y  = boxLidCenterY
        switch3Y  = boxLidCenterY - 750
        self.switch1 = PushButtonSwitchHole(Base.Vector(switchesX,switch1Y,switchesZ),height=switchesHeight)
        self.switch2 = PushButtonSwitchHole(Base.Vector(switchesX,switch2Y,switchesZ),height=switchesHeight)
        self.switch3 = PushButtonSwitchHole(Base.Vector(switchesX,switch3Y,switchesZ),height=switchesHeight)
        self.upperbox.cutlid(self.switch1.Hole())
        self.upperbox.cutlid(self.switch2.Hole())
        self.upperbox.cutlid(self.switch3.Hole())
        buzzerorig = Base.Vector(self.upperbox.InsideFrontX+(MM2Mils(32)/2.0),\
                                 boxLidCenterY,\
                                 self.upperbox.TopZ)
        self.buzzer = LargePiezo(name+"_buzzer",buzzerorig)
        self.upperbox.cutlid(self.buzzer.MountingHole(0,zMount=self.upperbox.TopZ,height=self.upperbox.WallThickness))
        self.upperbox.cutlid(self.buzzer.MountingHole(1,zMount=self.upperbox.TopZ,height=self.upperbox.WallThickness))
        self.upperbox.cutlid(self.buzzer.BodyHole(self.upperbox.TopZ,self.upperbox.WallThickness))
        fanrelaywirehole = Part.Face(Part.Wire(Part.makeCircle(\
                    375 / 2.0,\
                    Base.Vector(MOX+(ReflowToasterOven2Board.BoardLength()/2.0),\
                                boxLidCenterY,\
                                self.upperbox.WallThickness)))\
                     ).extrude(Base.Vector(0,0,-2*self.upperbox.WallThickness))
        self.upperbox.cutbox(fanrelaywirehole)
        self.lowerbox.cutbox(fanrelaywirehole)
        fanWireHole = Part.Face(Part.Wire(Part.makeCircle(\
                    250/2.0,\
                    Base.Vector(MOX+(ReflowToasterOven2Board.BoardLength()/2.0),\
                                boxLidCenterY-750,\
                                self.lowerbox.TopZ)))\
                      ).extrude(Base.Vector(0,0,-(125*6)))
        thermocoupleHole = Part.Face(Part.Wire(Part.makeCircle(\
                    125,\
                    Base.Vector(self.upperbox.InsideFrontX + \
                                self.upperbox.InsideLength + \
                                (self.upperbox.WallThickness / 2.0),\
                                boxLidCenterY,\
                                self.upperbox.TopZ/2.0),\
                    Base.Vector(1,0,0)))\
                    ).extrude(Base.Vector(self.upperbox.WallThickness,0,0))
        self.upperbox.cutbox(thermocoupleHole)
        usb_cutout = self.board.USBJackCutoutHole(self.upperbox.InsideFrontX)
        self.upperbox.cutbox(usb_cutout.extrude(\
                        Base.Vector(-2*self.upperbox.WallThickness,0,0)))
        relayX = self.lowerbox.InsideFrontX + MM2Mils(28.288) + 185 - 500
        relayY = ((self.lowerbox.InsideFrontY + self.lowerbox._insidewidth) - MM2Mils(32.085) - 875 + 250)
        relayOrig = Base.Vector(relayX,relayY,self.lowerbox.TopZ)
        self.relay = D2425(self.name+"_relay",relayOrig)
        acpowerentryOrig = Base.Vector(self.lowerbox.InsideFrontX - \
                                        (ACPowerEntry._flangeXdepth+\
                                         ACPowerEntry._flangeXoff+62.5),\
                                       self.lowerbox.InsideFrontY + \
                                            62.5,\
                                        self.lowerbox.origin.z + \
                                            self.lowerbox.WallThickness -\
                                            ACPowerEntry._flangewidthmax -\
                                            62.5)
        self.acpowerentry = ACPowerEntry(self.name+"_acpowerentry",\
                                         acpowerentryOrig,\
                                         orientation='vertical',\
                                         end='front')
        self.lowerbox.cutbox(self.acpowerentry.MountingHole(0,\
                                        depth=-self.lowerbox.WallThickness,\
                                        xPanelOff=self.acpowerentry._flangeXdepth))
        self.lowerbox.cutbox(self.acpowerentry.MountingHole(1,\
                                        depth=-self.lowerbox.WallThickness,\
                                        xPanelOff=self.acpowerentry._flangeXdepth))
        self.lowerbox.cutbox(self.acpowerentry.Cutout())
        backreceptOrig = Base.Vector(self.lowerbox.InsideFrontX + \
                                     self.lowerbox.InsideLength - \
                                     self.lowerbox.WallThickness,\
                                     self.lowerbox.InsideFrontY + 62.5,\
                                     self.lowerbox.origin.z + \
                                     self.lowerbox.WallThickness - \
                                     ACReceptacle._totalheight - 62.5)
        self.backrecept = ACReceptacle(self.name+"_backrecept",backreceptOrig,\
                            end='back')
        self.backreceptMHole = self.backrecept.MountHole(self.lowerbox._wallthickness*4,-self.lowerbox.WallThickness)
        self.lowerbox.cutbox(self.backreceptMHole)
        self.heatsink = HeatSink(self.name+"_heatsink",\
                    Base.Vector(self.lowerbox.OutsideFrontX,\
                    self.lowerbox.OutsideFrontY-(HeatSinkMain._length-self.lowerbox.OutsideWidth),\
                    self.lowerbox.TopZ-HeatSink._thickness))
        self.heatsink.cutfrom(self.lowerbox.LidHole(1,self.heatsink.origin.z,self.heatsink._thickness))
        self.heatsink.cutfrom(self.lowerbox.LidHole(2,self.heatsink.origin.z,self.heatsink._thickness))
        self.heatsink.cutfrom(self.lowerbox.LidHole(3,self.heatsink.origin.z,self.heatsink._thickness))
        self.heatsink.cutfrom(self.lowerbox.LidHole(4,self.heatsink.origin.z,self.heatsink._thickness))
        self.heatsink.cutfrom(self.relay.MountingHole(0,height=-(125*6)))
        self.heatsink.cutfrom(self.relay.MountingHole(1,height=-(125*6)))
        self.heatsink.cutfrom(fanWireHole)
    def show(self):
        self.upperbox.show()
        self.lowerbox.show()
        self.board.show()
        self.display.show()
        self.buzzer.show()
        self.relay.show()
        self.acpowerentry.show()
        self.backrecept.show()
        self.heatsink.show()
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name+"_board_standoff1")
        obj.Shape = self.board_standoff1
        obj.Label=self.name+"_board_standoff1"
        obj.ViewObject.ShapeColor=tuple([1.0,1.0,1.0])
        obj = doc.addObject("Part::Feature",self.name+"_board_standoff2")
        obj.Shape = self.board_standoff2
        obj.Label=self.name+"_board_standoff2"
        obj.ViewObject.ShapeColor=tuple([1.0,1.0,1.0])
        obj = doc.addObject("Part::Feature",self.name+"_board_standoff3")
        obj.Shape = self.board_standoff3
        obj.Label=self.name+"_board_standoff3"
        obj.ViewObject.ShapeColor=tuple([1.0,1.0,1.0])
        obj = doc.addObject("Part::Feature",self.name+"_board_standoff4")
        obj.Shape = self.board_standoff4
        obj.Label=self.name+"_board_standoff4"
        obj.ViewObject.ShapeColor=tuple([1.0,1.0,1.0])
        
if __name__ == '__main__':
    if "ReflowToasterOven" in App.listDocuments().keys():
        App.closeDocument("ReflowToasterOven")
    App.ActiveDocument=App.newDocument("ReflowToasterOven")
    doc = App.activeDocument()
    reflowcontrol = ReflowToasterOvenControlBox("reflowcontrol",Base.Vector(0,0,0))
    reflowcontrol.show()
    Gui.SendMsgToActiveView("ViewFit")
    Gui.activeDocument().activeView().viewIsometric()
    doc.Label="ReflowToasterOven"
    doc.saveAs("ReflowToasterOven.fcstd")
    sys.exit(1)
