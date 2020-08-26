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
#  Created       : Mon Aug 24 11:29:32 2020
#  Last Modified : <200825.1004>
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


import Part
from FreeCAD import Base
import FreeCAD as App
import os
import sys
sys.path.append(os.path.dirname(__file__))

from abc import ABCMeta, abstractmethod, abstractproperty

def MM2Mils(MM):
    return (MM/25.4)*1000
    
class ReflowToasterOven2Board(object):
    _boardWidth = MM2Mils(45.76)
    @staticmethod
    def BoardWidth():
        return ReflowToasterOven2Board._boardWidth
    _boardLength = MM2Mils(114.30)
    @staticmethod
    def BoardLength():
        return ReflowToasterOven2Board._boardLength
    _holes = { 1 : Base.Vector(4400,500,0), \
               2 : Base.Vector(4400,1300,0), \
               3 : Base.Vector(150,150,0), \
               4 : Base.Vector(150,1650.0) }
    _FeatherWidth = 900
    _FeatherLength = 2000
    _FeatherYUSB_Offset = 450-110
    _FeatherYBoardOffset = 500
    def StandOff(self,index,heightMM,above=False):
        boardbase = self.origin
        if above:
            boardbase = self.origin.add(Base.Vector(0,0,62.5))
        else:
            heightMM *= -1
        p = self._holes[index]
        return Part.Face(Part.Wire(Part.makeCircle(125,boardbase.add(p)))\
                         ).extrude(Base.Vector(0,0,MM2Mils(heightMM)))
    def mountingHole(self,index,height=62.5,standoffMM=0):
        #print("*** ReflowToasterOven2Board.mountingHole(%d,%g,%g)"%(index,height,standoffMM),file=sys.__stderr__)
        p = self._holes[index]
        #print("*** ReflowToasterOven2Board.mountingHole(): p is (%g,%g,%g)"%(p.x,p.y,p.z),file=sys.__stderr__)
        p = p.add(Base.Vector(0,0,MM2Mils(standoffMM)))
        #print("*** ReflowToasterOven2Board.mountingHole(): p (add standoffMM) is (%g,%g,%g)"%(p.x,p.y,p.z),file=sys.__stderr__)
        holeorig = self.origin.add(p)
        #print("*** ReflowToasterOven2Board.mountingHole(): holeorig is (%g,%g,%g)"%(holeorig.x,holeorig.y,holeorig.z),file=sys.__stderr__)
        return Part.Face(Part.Wire(Part.makeCircle(62.5,holeorig))\
                        ).extrude(Base.Vector(0,0,height))
    def USBJack(self):
        return Part.makePlane(300,220,\
                              self.origin.add(Base.Vector(-40,\
                          self._FeatherYUSB_Offset+self._FeatherYBoardOffset,\
                          62.5 + 196.5 + 100 + 62.5))\
                    ).extrude(Base.Vector(0,0,110))
    def USBJackCutout(self,xPanel):
        cutoutOrig = self.origin.add(Base.Vector(-40,\
                          self._FeatherYUSB_Offset+self._FeatherYBoardOffset,\
                          62.5 + 196.5 + 100 + 62.5))
        cutoutOrig = Base.Vector(xPanel,cutoutOrig.y+220,cutoutOrig.z)
        return Part.makePlane(110,220,cutoutOrig,Base.Vector(1,0,0))
    def USBJackCutoutHole(self,xPanel):
        cutoutOrig = self.origin.add(Base.Vector(-40,\
                          self._FeatherYUSB_Offset+self._FeatherYBoardOffset,\
                          62.5 + 196.5 + 100 + 62.5))
        cutoutOrig = Base.Vector(xPanel,cutoutOrig.y+110,cutoutOrig.z+55)
        return Part.Face(Part.Wire(Part.makeCircle(175,cutoutOrig,Base.Vector(1,0,0))))
    def show(self):
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name+"_board")
        obj.Shape = self.board
        obj.Label=self.name+"_board"
        obj.ViewObject.ShapeColor=tuple([210/255.0,180/255.0,140/255.0])
        obj = doc.addObject("Part::Feature",self.name+"_h1")
        obj.Shape = self.h1
        obj.Label=self.name+"_h1"
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
        obj = doc.addObject("Part::Feature",self.name+"_h2")
        obj.Shape = self.h2
        obj.Label=self.name+"_h2"
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
        obj = doc.addObject("Part::Feature",self.name+"_feather")
        obj.Shape = self.feather
        obj.Label=self.name+"_feather"
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
        obj = doc.addObject("Part::Feature",self.name+"_usbjack")
        obj.Shape = self.usbjack
        obj.Label=self.name+"_usbjack"
        obj.ViewObject.ShapeColor=tuple([.85,.85,.85])
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        self.board = Part.makePlane(self._boardLength,self._boardWidth,self.origin).extrude(Base.Vector(0,0,62.5))
        self.h1 = Part.makePlane(1600,100,self.origin.add(Base.Vector(200,500,62.5))).extrude(Base.Vector(0,0,296.5))
        self.h2 = Part.makePlane(1200,100,self.origin.add(Base.Vector(600,1300,62.5))).extrude(Base.Vector(0,0,296.5))
        self.feather = Part.makePlane(self._FeatherLength,self._FeatherWidth,\
                                      self.origin.add(Base.Vector(0,self._FeatherYBoardOffset,\
                                              62.5 + 196.5 + 100))).extrude(Base.Vector(0,0,62.5))
        self.usbjack = self.USBJack()                                                        
        self.board = self.board.cut(self.mountingHole(1))
        self.board = self.board.cut(self.mountingHole(2))
        self.board = self.board.cut(self.mountingHole(3))
        self.board = self.board.cut(self.mountingHole(4))


if __name__ == '__main__':
    if "ReflowToasterOven2Board" in App.listDocuments().keys():
        App.closeDocument("ReflowToasterOven2Board")
    App.ActiveDocument=App.newDocument("ReflowToasterOven2Board")
    doc = App.activeDocument()
    board = ReflowToasterOven2Board("b",Base.Vector(0,0,0))
    board.show()
    Gui.SendMsgToActiveView("ViewFit")
    Gui.activeDocument().activeView().viewIsometric()                
