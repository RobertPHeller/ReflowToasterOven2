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
#  Created       : Thu Aug 27 07:48:14 2020
#  Last Modified : <200827.0854>
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
    
class Button(object):
    _bodywidthheight = 250
    _bodyheight      = 125
    _buttonwh        = 125
    _buttonheight    = 125
    def show(self):
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name+"_body")
        obj.Shape = self.body
        obj.Label=self.name+"_body"
        obj.ViewObject.ShapeColor=tuple([.1,.1,.1])
        obj = doc.addObject("Part::Feature",self.name+"_button")
        obj.Shape = self.button
        obj.Label=self.name+"_button"
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,1.0])
    def ButtonHole(self,zPanel,panelThick,radius=125):
        holeOrig = Base.Vector(self.buttonX,self.buttonY,zPanel)
        return Part.Face(Part.Wire(Part.makeCircle(radius,holeOrig))\
                        ).extrude(Base.Vector(0,0,panelThick))
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        self.buttonX = self.origin.x
        self.buttonY = self.origin.y
        o = Base.Vector(self.buttonX - (self._bodywidthheight/2),\
                        self.buttonY - (self._bodywidthheight/2),\
                        self.origin.z)
        self.body = Part.makePlane(self._bodywidthheight,\
                                   self._bodywidthheight,o\
                                  ).extrude(Base.Vector(0,0,self._bodyheight))
        o = Base.Vector(self.buttonX - (self._buttonwh/2),\
                        self.buttonY - (self._buttonwh/2),\
                        self.origin.z+self._bodyheight)
        self.button = Part.makePlane(self._buttonwh,self._buttonwh,o\
                                    ).extrude(Base.Vector(0,0,\
                                                          self._buttonheight))
        

class ButtonBoard(object):
    _boardWidth = 1600
    _boardHeight = 700
    _boardThick = 62.5
    _mholerad   = 62.5
    def show(self):
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name+"_board")
        obj.Shape = self.board
        obj.Label=self.name+"_board"
        obj.ViewObject.ShapeColor=tuple([210/255.0,180/255.0,140/255.0])
        self.button1.show()
        self.button2.show()
        self.button3.show()
    def ButtonHole(self,i,zPanel,panelThick,radius=125):
        if i == 1:
            return self.button1.ButtonHole(zPanel,panelThick,radius)
        elif i == 2:
            return self.button2.ButtonHole(zPanel,panelThick,radius)
        elif i == 3:
            return self.button3.ButtonHole(zPanel,panelThick,radius)
        else:
            return None
    def MountHole(self,i,zPanel,panelThick):
        if i == 1:
            mh = self.mh1
        elif i == 2:
            mh = self.mh2
        else:
            return None
        mh = Base.Vector(mh.x,mh.y,zPanel)
        return Part.Face(Part.Wire(Part.makeCircle(self._mholerad,mh))\
                        ).extrude(Base.Vector(0,0,panelThick))
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        self.board = Part.makePlane(self._boardHeight,self._boardWidth,\
                                    self.origin\
                                   ).extrude(Base.Vector(0,0,self._boardThick))
        centerY = self.origin.y + (self._boardWidth / 2)
        buttonX = self.origin.x + (self._boardHeight / 2)
        if buttonX % 100 != 0:
            buttonX -= 50
        button2Y = centerY
        button1Y = centerY - 300
        button3Y = centerY + 300
        mhX = buttonX
        mh1Y = button1Y - 300
        mh2Y = button3Y + 300
        self.button1 = Button(self.name+"_button1",\
                              Base.Vector(buttonX,button1Y,\
                                          self.origin.z+self._boardThick))
        self.button2 = Button(self.name+"_button2",\
                              Base.Vector(buttonX,button2Y,\
                                          self.origin.z+self._boardThick))
        self.button3 = Button(self.name+"_button3",\
                              Base.Vector(buttonX,button3Y,\
                                          self.origin.z+self._boardThick))
        self.mh1 = Base.Vector(mhX,mh1Y,self.origin.z)
        self.mh2 = Base.Vector(mhX,mh2Y,self.origin.z)
        self.board = self.board.cut(Part.Face(Part.Wire(\
                                        Part.makeCircle(self._mholerad,\
                                                        self.mh1))\
                                  ).extrude(Base.Vector(0,0,self._boardThick)))
        self.board = self.board.cut(Part.Face(Part.Wire(\
                                        Part.makeCircle(self._mholerad,\
                                                        self.mh2))\
                                  ).extrude(Base.Vector(0,0,self._boardThick)))


if __name__ == '__main__':
    App.ActiveDocument=App.newDocument("ButtonBoard")
    doc = App.activeDocument()
    buttonBoard = ButtonBoard("buttonboard",Base.Vector(0,0,0))
    buttonBoard.show()
    Gui.SendMsgToActiveView("ViewFit") 
    Gui.activeDocument().activeView().viewIsometric()
