#*****************************************************************************
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Robert Heller
#  Created       : Mon Aug 24 22:33:16 2020
#  Last Modified : <200824.2321>
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
    
class TFT_Display(object):
    _boardlength = 2200
    _boardwidth  = 1350
    _holes       = { 1 : Base.Vector(100,100,0),\
                     2 : Base.Vector(2100,100,0),\
                     3 : Base.Vector(2100,1250,0),\
                     4 : Base.Vector(100,1250,0) }
    _displaylength = 1400
    _displaywidth  = 1125
    _displayxoff   =  400
    _displayyoff   =  100
    @staticmethod
    def BoardLength():
        return TFT_Display._boardlength
    @staticmethod
    def BoardWidth():
        return TFT_Display._boardwidth
    def show(self):
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name+"_board")
        obj.Shape = self.board
        obj.Label=self.name+"_board"
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,1.0])
        obj = doc.addObject("Part::Feature",self.name+"_display")
        obj.Shape = self.display
        obj.Label=self.name+"_display"
        obj.ViewObject.ShapeColor=tuple([1.0,1.0,1.0])
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        self.board = Part.makePlane(self._boardlength,self._boardwidth,\
                                    self.origin).extrude(Base.Vector(0,0,100))
        
        self.display = Part.makePlane(self._displaylength,self._displaywidth,\
                           self.origin.add(Base.Vector(self._displayxoff,\
                                                       self._displayyoff,\
                                                       100))\
                           ).extrude(Base.Vector(0,0,200))
                                      
        for i in [1,2,3,4]:
            p = self._holes[i]
            horig = self.origin.add(p)
            self.board = self.board.cut(Part.Face(Part.Wire(Part.makeCircle(62.5,horig))).extrude(Base.Vector(0,0,100)))
    def MountingHole(self,index,zMount,height):
        p = self._holes[index]
        horig = self.origin.add(Base.Vector(p.x,p.y,0))
        horig = Base.Vector(horig.x,horig.y,zMount)
        return Part.Face(Part.Wire(Part.makeCircle(62.5,horig))\
                        ).extrude(Base.Vector(0,0,height))
    def DisplayCutout(self,zPanel,height):
        curoutOrig = self.origin.add(Base.Vector(self._displayxoff,\
                                                       self._displayyoff,\
                                                       0))
        curoutOrig = Base.Vector(curoutOrig.x,curoutOrig.y,zPanel)
        return Part.makePlane(self._displaylength,self._displaywidth,\
                           curoutOrig).extrude(Base.Vector(0,0,height))        
