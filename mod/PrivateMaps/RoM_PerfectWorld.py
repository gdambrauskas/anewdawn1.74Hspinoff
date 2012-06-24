##############################################################################
## File: PerfectWorld.py version 1.13
## Author: Rich Marinaccio
## Copyright 2007 Rich Marinaccio
##
## This map script for Civ4 generates a random, earth-like map, usually with a
## 'New World' with no starting locations that can only be reached with
## ocean going technology. Though great pains are taken to accurately simulate
## landforms and climate, the goal must be to make unpredictible, beautiful
## looking maps that are fun to play on.
## 
## -- Summary of creation process: --
## First, a random heightfield is created using midpoint displacement. The
## resulting altitudes are then modified by a plate tectonics scheme that
## grows random plates and raises the altitudes near the plate borders to
## create mountain ranges and island chains.
##
## In generating the plot types from a heightmap, I had found that using
## peaks for high altitude and land for less altitude created large clusters
## of peaks, surrounded by a donut of hills, surrounded again by a donut of
## land. This looked absolutely terrible for Civ, so I made it such that
## peaks and hills are determined by altitude *differences* rather than by
## absolute altitude. This approach looks much better and more natural.
##
## The terrain generator gives the other needed visual cues to communicate
## altitude. Since air temperature gets colder with altitude, the peaks will
## be plots of ice and tundra, even near the equator, if the altitude
## is high enough. Prevailing winds, temperature and rainfall are all simulated
## in the terrain generator. You will notice that the deserts and rainforests
## are where they should be, as well as rain shadows behind mountain ranges.
##
## Rivers and lakes are also generated from the heightmap and follow accurate
## drainage paths, although with such a small heightmap some randomness needs
## to be thrown in to prevent rivers from being merely straight lines.
##
## Map bonuses are placed following the XML Rules but slightly differently than
## the default implimentation to better accomodate this map script.
##
## I've always felt that the most satisfying civ games are the ones that
## provide a use for those explorers and caravels. Though the map generator
## does not explicitly create a 'New World', it will take advantage of any
## continents that can serve that purpose. No starting locations will be placed
## on these continents. Therefore, the likelyhood of a significant new world
## is very high, but not guaranteed. It might also come in the form of multiple
## smaller 'New Worlds' rather than a large continent.
##
##############################################################################
## Version History
## 1.13 - Fixed a bug where starting on a goody hut would crash the game.
## Prevented start plots from being on mountain peaks. Changed an internal
## distance calculation from a straight line to a path distance, improving
## start locations somewhat. Created a new tuning variable called
## DesertLowTemp. Since deserts in civ are intended to be hot deserts, this
## variable will prevent deserts from appearing near the poles where the
## desert texture clashes horribly with the tundra texture.
##
## 1.12 - Found a small bug in the bonus placer that gave bonuses a minimum
## of zero, this is why duel size maps were having so much trouble.
##
## 1.11 - limited the features mixing with bonuses to forests only. This
## eliminates certain undesireable effects like floodplains being erased by
## or coinciding with oil or incense, or corn appearing in jungle.
##
## 1.10 - Wrapped all map constants into a class to avoid all those
## variables being loaded up when PW is not used. Also this makes it a
## little easier to change them programatically. Added two in-game options,
## New World Rules and Pangaea Rules. Added a tuning variable that allows
## bonuses with a tech requirement to co-exist with features, so that the
## absence of those features does not give away their location.
##
## 1.09 - Fixed a starting placement bug introduced in 1.07. Added a tuning
## variable to turn off 'New world' placement.
##
## 1.08 - Removed the hemispheres logic and replaced it with a simulated meteor
## shower to break up pangeas. Added a tuning variable to allow pangeas.
##
## 1.07 - Placing lakes and harbors after river placement was not updating river
## crossings. Resetting rivers after lake placement should solve this. Fixed a
## small discrepancy between Python randint and mapRand to make them behave the
## same way. Bonuses of the same bonus class, when forced to appear on the
## same continent, were sometimes crowding each other off the map. This was
## especially problematic on the smaller maps. I added some additional, less
## restrictive, passes to ensure that every resource has at least one placement
## unless the random factors decide that none should be placed. Starting plot
## normalization now will place food if a different bonus can not be used due
## to lack of food. Changed heightmap generation to more likely create a
## new world.
##
## 1.06 - Overhauled starting positions and resource placement to better
## suit the peculiarities of PerfectWorld
##
## 1.05 - Fixed the Mac bug and the multi-player bug.
##
## 1.04a - I had unfairly slandered getMapRand in my comments. I had stated
## that the period was shortened unnecessarily, which is not the case.
##
## 1.04 - Added and option to use the superior Python random number generator
## or the getMapRand that civ uses. Made the number of rivers generated tunable.
## Fixed a bug that prevented floodplains on river corners. Made floodplains
## in desert tunable.
##
## 1.03a - very minor change in hope of finding the source of a multi-player
## glitch.
##
## 1.03 - Improved lake generation. Added tuning variables to control some
## new features. Fixed some minor bugs involving the Areamap filler
## and fixed the issue with oasis appearing on lakes. Maps will now report
## the random seed value that was used to create them, so they can be easily
## re-created for debugging purposes.
##
## 1.02 - Fixed a bug that miscalculated the random placing of deserts. This
## also necessitated a readjustment of the default settings.
##
## 1.01 - Added global tuning variables for easier customization. Fixed a few
## bugs that caused deserts to get out of control.
##
from CvPythonExtensions import *
import CvUtil
import CvMapGeneratorUtil 


from array import array
from random import random,randint,seed
import math
import sys
import operator
import time

class MapConstants :
    def __init__(self):
        return
    def initialize(self):
##############################################################################
## GLOBAL TUNING VARIABLES: Change these to customize the map results

        #How much land versus total map squares. Keep in mind that values higher than
        #0.25 will greatly minimize the chance of a significant 'new world'.
        self.LandPercent = 0.26

        #If this variable is set to False, a shower of colossal meteors will attempt to
        #break up any pangea-like continents. Setting this variable to True will allow
        #pangeas to sometimes occur and should be greatly favored by any dinosaurs
        #that might be living on this planet at the time.
        self.AllowPangeas = False

        #This variable can be used to turn off 'New world' logic and place starting
        #positions anywhere in the world. For some mods, a new world doesn't make
        #sense.
        self.AllowNewWorld = True

        #This variable controls wether unrevealed bonuses can coexist with features that
        #are normally not allowed in the XML rules. The problem with the normal rule
        #with regard to PW is that the conspicuous absence of trees or floodplain can
        #expose the existence of a future resource. This is optional in case future
        #graphics sets do not accomodate the combination of all bonuses with all features,
        #but with the normal Civ game this causes no problems.
        self.HiddenBonusInForest = True
        
        #How many map squares will be above peak threshold and thus 'peaks'.
        self.PeakPercent = 0.030

        #How many map squares will be above hill threshold and thus 'hills' unless
        #they are also above peak threshold in which case they will be 'peaks'.
        self.HillPercent = 0.09

        #How many map squares will be below desert rainfall threshold. In this case,
        #rain levels close to zero are very likely to be desert, while rain levels close
        #to the desert threshold will more likely be plains.
        self.DesertPercent = 0.06

        #How many map squares will be below plains rainfall threshold. Rain levels close
        #to the desert threshold are likely to be plains, while those close to the plains
        #threshold are likely to be grassland. Any rain above plains threshold will be
        #grassland.
        self.PlainsPercent = 0.14

		#How many map squares will be below marsh rainfall threshold. Rain levels close
        #to the plains threshold are likely to be marsh, while those close to the marsh
        #threshold are likely to be plains. Any rain above marsh threshold will be
        #plains.
        self.MarshPercent = 0.10

		
        #---The following variables are not based on percentages. Because temperature
        #---is so strongly tied to latitude, using percentages for things like ice and
        #---tundra leads to very strange results if most of the worlds land lies near
        #---the equator

        #What temperature will be considered cold enough to be ice. Temperatures range
        #from coldest 0.0 to hottest 1.0.
        self.IceTemp = .15

        #What temperature will be considered cold enough to be tundra. Temperatures range
        #from coldest 0.0 to hottest 1.0.
        self.TundraTemp = .25

        #What temperature will be too cold for desert. Desert dryness here will
        #convert to plains.
        self.DesertLowTemp = .35

# Rise of Mankind 2.5		
        self.MarshLowTemp = .30
# Rise of Mankind 2.5
		
        #Hotter than this temperature will be considered deciduous forest, colder will
        #be evergreen forest.Temperatures range from coldest 0.0 to hottest 1.0.
        self.ForestTemp = .4

        #What temperature will be considered hot enough to be jungle. Temperatures range
        #from coldest 0.0 to hottest 1.0.
        self.JungleTemp = .6

        #---The following variables modify various map details.

        #The percent chance that an oasis may appear in desert. A tile must be desert and
        #surrounded on all sides by desert.
        self.OasisChance = .08

        #How many squares are added to a lake for each unit of river length flowing
        #into it.
        self.LakeSizePerRiverLength = .75

        #This value modifies LakeSizePerRiverLength when a lake begins in desert
        self.DesertLakeModifier = .60

        #This value is used to decide if enough water has accumulated to form a river.
        #A lower value creates more rivers over the entire map.
        self.RiverThreshold = 2.0

        #This value is multiplied by the river threshold to determine if there is
        #enough water to flood a river bank and create a floodplain. A value of 1.0
        #will cause every river to flood, higher values will cause fewer river tiles
        #to flood. 
        self.FloodPlainThreshold = 1.8

        #---I experimented alot with these rain smoothing variables, and decided that
        #---they do not improve the map. Therefore, I have set them to zero. Go ahead
        #---and play with these if you want, but their main effect is to diminish
        #---all the hard work that went into simulating climate.

        #How much noise to add to the rainfall map. This helps break up harsh lines
        #near the rainy latitudes. 
        self.RainNoise = .0

        #Smoothing factor for the rain map. Works with RainNoise to avoid harsh
        #artifacts due to rainfall wind zone changes, especially near the equator.
        self.RainSmoothingFactor = .0

        #---These values are for evaluating starting locations

        #The following values are used for assigning starting locations. For now,
        #they have the same ratio that is found in CvPlot::getFoundValue
        self.CommerceValue = 20
        self.ProductionValue = 40
        self.FoodValue = 10

        #Coastal cities are important, how important is determined by this
        #value.
        self.CoastalCityValueBonus = 1.3

        #Decides whether to use the Python random generator or the one that is
        #intended for use with civ maps. The Python random has much higher precision
        #than the civ one. 53 bits for Python result versus 16 for getMapRand. The
        #rand they use is actually 32 bits, but they shorten the result to 16 bits.
        #However, the problem with using the Python random is that it may create
        #syncing issues for multi-player now or in the future, therefore it must
        #be optional.
        self.UsePythonRandom = True

        return
    def initInGameOptions(self):
        gc = CyGlobalContext()
        mmap = gc.getMap()
        #New World Rules
        selectionID = mmap.getCustomMapOption(0)
        if selectionID == 1:
            self.AllowNewWorld = False
        #Pangaea Rules
        selectionID = mmap.getCustomMapOption(1)
        if selectionID == 1:
            self.AllowPangeas = True

        return
mc = MapConstants()

class PerfectWorldRandom :
    def __init__(self):
        return
    def seed(self):
        #Python randoms are not usable in network games.
        if mc.UsePythonRandom:
            self.usePR = True
        else:
            self.usePR = False
        if self.usePR and CyGame().isNetworkMultiPlayer():
            print "Detecting network game. Setting UsePythonRandom to False."
            self.usePR = False
        if self.usePR:
            # Python 'long' has unlimited precision, while the random generator
            # has 53 bits of precision, so I'm using a 53 bit integer to seed the map!
            seed() #Start with system time
            seedValue = randint(0,9007199254740991)
            seed(seedValue)
            print "Random seed (Using Python rands) for this map is %(s)20d" % {"s":seedValue}
            
##            seedValue = 70450052590418
##            seed(seedValue)
##            print "Pre-set seed (Using Pyhon rands) for this map is %(s)20d" % {"s":seedValue}
        else:
            gc = CyGlobalContext()
            self.mapRand = gc.getGame().getMapRand()
            
            seedValue = self.mapRand.get(65535,"Seeding mapRand - PerfectWorld.py")
            self.mapRand.init(seedValue)
            print "Random seed (Using getMapRand) for this map is %(s)20d" % {"s":seedValue}
            
##            seedValue = 56870
##            self.mapRand.init(seedValue)
##            print "Pre-set seed (Using getMapRand) for this map is %(s)20d" % {"s":seedValue}
        return
    def random(self):
        if self.usePR:
            return random()
        else:
            #This formula is identical to the getFloat function in CvRandom. It
            #is not exposed to Python so I have to recreate it.
            fResult = float(self.mapRand.get(65535,"Getting float -PerfectWorld.py"))/float(65535)
#            print fResult
            return fResult
    def randint(self,rMin,rMax):
        #if rMin and rMax are the same, then return the only option
        if rMin == rMax:
            return rMin
        #returns a number between rMin and rMax inclusive
        if self.usePR:
            return randint(rMin,rMax)
        else:
            #mapRand.get() is not inclusive, so we must make it so
            return rMin + self.mapRand.get(rMax + 1 - rMin,"Getting a randint - PerfectWorld.py")
#Set up random number system for global access
PWRand = PerfectWorldRandom()
    
class Heightmap :
    def __init__(self):
        return
    def GenerateMap(self,mapWidth,mapHeight,landPercent):
        if landPercent < 0 or landPercent > 1:
            raise ValueError, "landPercent must be between 0.0 and 1.0"

        #This is necessary because the doRiver function requires one recursion
        #level for each unit of river length. Since we don't want to limit our
        #river length to 25, we must set the recursion limit somewhat longer
        sys.setrecursionlimit( 100)
        
        self.mapHeight = mapHeight
        self.mapWidth = mapWidth
        self.chunkSize = self.FindIdealChunkSize(mapWidth,mapHeight)
        self.landPercent = landPercent

        #initial level, this will be changed to fit map
        self.seaLevel = .45

        self.map = array('d')
        #initialize map with zeros
        for i in range(0,mapHeight*mapWidth):
            self.map.append(0.0)

        self.GenerateHeightField()
        self.NormalizeHeightMap()
        
#        PrintHeightMap(self)
        
        self.PerformTectonics()
#        self.SinkMiddle()
        self.NormalizeHeightMap()
#        PrintHeightMap(self)
        self.FindSeaLevel()
        self.GenerateFinalPlotTypes()
        self.FillInLakes()
        
#        PrintHeightMap(self)
#        print self.seaLevel,self.flatLevel,self.hillLevel
#        PrintPlateMap(self)
#        PrintInfluMap(self)
                   

    def FindIdealChunkSize(self,Width,Height):
        #The ideal chunksize is the largest power of 2 that is smaller than
        #Width/4, such that the Height is also evenly divisible by chunkSize.
        #If the chunkSize is too small (< Width/16) the map will be dotted with
        #tiny islands which looks bad and is not the intent of this map script
        #so an exception is thrown.
        targetSize = Width/4
        s = targetSize
        while s >= Width/16:
#            print "s = " + str(s)
            p = 1
            while 2**p < targetSize:
#                print "2**p = " + str(2**p)
                if (2**p)*2 > s:
#                    print "Width(" + str(Width) + ") % " + str(2**p) + " = " + str(Width % (2**p))
#                    print "Height(" + str(Height) + ") % " + str(2**p) + " = " + str(Height % (2**p))
                    if Width % (2**p) == 0 and Height % (2**p) == 0:
                        print "final chunkSIze = " + str(2**p)
                        return 2**p
                p += 1
            s = s/2
        errorString = \
        "This map script has fairly strict size requirements. " + \
        "Dimensions that are powers of 2 should guarantee compliance. " + \
        "Specifically, map must be evenly divisible, in Width and in " + \
        "Height, by a power of two that is at least mapWidth/16."
                    
        raise ValueError,errorString
              
              
        return 
    def GenerateHeightField(self):
        #get number of peaks needed to approximate land percentage
        numberOfPeaks = int(round((self.mapHeight/self.chunkSize) * (self.mapWidth/self.chunkSize) * self.landPercent))        

        yChunks = self.mapHeight/self.chunkSize
        xChunks = self.mapWidth/self.chunkSize
        gapSize = xChunks/6
        print "xChunks = %(xc)d, gapSize = %(gs)d" % {"xc":xChunks,"gs":gapSize}
        peakList = list()
        for n in range(numberOfPeaks):
            peakFound = False
            iterations = 0
            while not peakFound:
                iterations += 1
                x = PWRand.randint(0,xChunks - 1) * self.chunkSize
                y = PWRand.randint(1,yChunks - 2) * self.chunkSize
                peakFound = True
                for nn in peakList:
                    xx,yy = nn
                    #no repeats 
                    if x == xx and y == yy:
                        peakFound = False
                if iterations >= 200:
                    print "Can't find legal peaks"
                    peakFound = True
                if peakFound:
                    i = self.GetIndex(x,y)
                    nn = x,y
                    peakList.append(nn)
                    self.map[i] = 1.0
        #now generate rest of map using midpoint displacement. For rectangular
        #heightmaps this requires two passes
        currentGrain = float(self.chunkSize)
        while currentGrain > 1:
            for y in range(0,self.mapHeight,int(currentGrain)):
                for x in range(0,self.mapWidth,int(currentGrain)):
                    #h is scalar for random displacement
                    h = currentGrain/float(self.chunkSize)
                    #calculate points one currentGrain/2 to the right and
                    #also one down
                    mid = self.GetIndex(x + int(currentGrain/2),y)
                    left = self.GetIndex(x,y)
                    right = self.GetIndex(x + int(currentGrain),y)
                    self.map[mid] = (self.map[left] + self.map[right])/2 \
                        + (h*PWRand.random() - h/2.0)
                    #also add heuristic to left and right to help reduce artifacts
                    self.map[left] += (h*PWRand.random() - h/2.0)
                    self.map[right] += (h*PWRand.random() - h/2.0)
                    #now do down
                    mid = self.GetIndex(x,y + int(currentGrain/2))
                    top = self.GetIndex(x,y)
                    bottom = self.GetIndex(x,y + int(currentGrain))
                    self.map[mid] = (self.map[top] + self.map[bottom])/2 \
                        + (h*PWRand.random() - h/2.0)
                    #add heuristic to bottom, top is handled above (top = left)
                    self.map[bottom] += (h*PWRand.random() - h/2.0)
            
            for y in range(0,self.mapHeight,int(currentGrain)):
                for x in range(0,self.mapWidth,int(currentGrain)):
                    #h is scalar for random displacement
                    h = (currentGrain/2)/float(self.chunkSize)
                    #calculate points one currentGrain/2 to the right and
                    #also one down
                    mid = self.GetIndex(x + int(currentGrain/2),y + int(currentGrain/2))
                    top = self.GetIndex(x + int(currentGrain/2),y)
                    bottom = self.GetIndex(x + int(currentGrain/2),y + int(currentGrain))
                    left = self.GetIndex(x,y + int(currentGrain/2))
                    right = self.GetIndex(x+int(currentGrain),y + int(currentGrain/2))
                    self.map[mid] = (self.map[left] + self.map[right] \
                        + self.map[top] + self.map[bottom])/4 \
                        + (h*PWRand.random() - h/2.0)
            currentGrain = currentGrain/2
        return
    def GetIndex(self,x,y):
        #wrap in both directions for the purpose of generating heights. This
        #allows us to continue using even numbered sizes for maps, and should
        #not impact the desired result.
        dy = y; dx = x
        if y >= self.mapHeight:
            dy = y - self.mapHeight
        elif y < 0:
            dy = self.mapHeight + y
        if x >= self.mapWidth:
            dx = x - self.mapWidth
        elif x < 0:
            dx = self.mapWidth + x
            
        index = dy*self.mapWidth + dx
        return index
    def SinkMiddle(self):
        halfGap = self.mapWidth/10
        for y in range(self.mapHeight):
            for x in range(self.mapWidth/2 - halfGap,self.mapWidth/2 + halfGap):
                i = self.GetIndex(x,y)
                scaler = 0.75 + (float(abs(x - self.mapWidth/2))/float(halfGap) * 0.25)
#                print "distance to middle = %(d)d, scaler = %(s)f" % {"d":abs(x - self.mapWidth/2),"s":scaler}
                self.map[i] *= scaler
        return
    def PerformTectonics(self):
        self.plateMap = array('i')
        doneMap = array('b') #use a little extra memory to speed this operation up
        #initialize map with zeros
        for i in range(0,self.mapHeight*self.mapWidth):
            self.plateMap.append(0)
            doneMap.append(False)
        
        #seed mapWidth/4 number of plates
        numberOfPlates = self.mapWidth/4
        for x in range(1,numberOfPlates + 1):
            i = PWRand.randint(0,self.mapWidth*self.mapHeight - 1)
            self.plateMap[i] = x

        #grow plates
        doneMapFull = False
        while not doneMapFull:
            doneMapFull = True
            for y in range(0,self.mapHeight):
                for x in range(0,self.mapWidth):
                    i = self.GetIndex(x,y)
                    if self.plateMap[i] != 0 and doneMap[i] == False:
                        up = self.GetIndex(x,y-1)
                        down = self.GetIndex(x,y+1)
                        left = self.GetIndex(x-1,y)
                        right = self.GetIndex(x+1,y)
                        done = True
                        if self.plateMap[up] == 0:
                            done = False
                            if PWRand.random() < .5:
                                self.plateMap[up] = self.plateMap[i]
                        if self.plateMap[down] == 0:
                            done = False
                            if PWRand.random() < .5:
                                self.plateMap[down] = self.plateMap[i]
                        if self.plateMap[left] == 0:
                            done = False
                            if PWRand.random() < .5:
                                self.plateMap[left] = self.plateMap[i]
                        if self.plateMap[right] == 0:
                            done = False
                            if PWRand.random() < .5:
                                self.plateMap[right] = self.plateMap[i]
                        if done == True:
                            doneMap[i] = True
                        else:
                            doneMapFull = False
                            
        #Stagger the plates somewhat to add interest
        for i in range(0,self.mapHeight*self.mapWidth):
            self.map[i] *= 0.5 + (float(self.plateMap[i]) * 0.01)
                            
        #Now that the plates are grown, we must decide how the plate boundaries
        #will effect our map. Basically, plots away from plate boundaries will
        #cause some sinking, while plots closer to them while rise. After
        #multiplying, the main heightmap will be re-normalized.
        sinkValue = 1.5
        peakValue = 2.0
        searchRadius = 3
        self.influMap = array('d')
        #initialize map with sinkValue
        for i in range(0,self.mapHeight*self.mapWidth):
            self.influMap.append(sinkValue)

        for y in range(self.mapHeight):
            for x in range(self.mapWidth):
                distance = self.FindDistanceToPlateBoundary(x,y,searchRadius)
                i = self.GetIndex(x,y)
                if distance == 0.0:
                    #not found in search radius
                    self.influMap[i] = sinkValue
                else:
                    self.influMap[i] = self.GetInfluFromDistance(sinkValue,peakValue,searchRadius,distance)

        #next step is to multiply the influence map by the original heightmap
        for y in range(self.mapHeight):
            for x in range (self.mapWidth):
                i = self.GetIndex(x,y)
                self.map[i] *= self.influMap[i]
                            
        return
    def GetInfluFromDistance(self,sinkValue,peakValue,searchRadius,distance):
        influence = peakValue
        maxDistance = math.sqrt(pow(float(searchRadius),2) + pow(float(searchRadius),2))
        #minDistance = 1.0
        influence -= ((peakValue - sinkValue)* (distance - 1.0))/(maxDistance - 1.0)
        return influence
    def FindDistanceToPlateBoundary(self,x,y,searchRadius):
        minDistance = 10.0
        i = self.GetIndex(x,y)
        for yy in range(y - searchRadius,y + searchRadius):
            for xx in range(x - searchRadius,x + searchRadius):
                ii = self.GetIndex(xx,yy)
                if self.plateMap[i] != self.plateMap[ii]:
                    distance = math.sqrt(pow(float(xx-x),2) + pow(float(yy-y),2))
                    if distance < minDistance:
                        minDistance = distance
                           
        if minDistance == 10.0:
            return 0.0
        
        return minDistance
    def FindSeaLevel(self):
        flatCount = 0
#        hillCount = 0
#        peakCount = 0
        seaTolerance = .03
#        flatTolerance = .018
#        hillTolerance = .002
        
        totalCount = self.mapHeight*self.mapWidth
        seaInTolerance = False
#        flatInTolerance = False
#        hillInTolerance = False
        seaLevelChange = 0.15
#        flatLevelChange = 0.15
#        hillLevelChange = 0.15
#        hillPercent = self.landPercent/2.0
#        peakPercent = hillPercent/3.0
        #each type needs it's own loop
        #land(flat)
#        print "flat"
        iterations = 0
        while not seaInTolerance:
            iterations += 1
            flatCount = 0
            if iterations > 1000:
                break #close as can be achieved
            for i in range(self.mapHeight*self.mapWidth):
                if self.map[i] > self.seaLevel:
                    flatCount += 1
            currentLandPercent = float(flatCount)/float(totalCount)
#            print "currentPercent = " + str(currentLandPercent), "desired = " + str(self.landPercent)
            if currentLandPercent < self.landPercent + seaTolerance and \
               currentLandPercent > self.landPercent - seaTolerance:
                seaInTolerance = True
            elif currentLandPercent < self.landPercent - seaTolerance:
                self.seaLevel -= seaLevelChange
            else:
                self.seaLevel += seaLevelChange
            if not seaInTolerance:
#                print "sealevel = " + str(self.seaLevel), "change = " + str(seaLevelChange)
                seaLevelChange = seaLevelChange/2.0
        #hill
        return
    def NormalizeHeightMap(self):
        #find highest and lowest points
        maxAlt = 0.0
        minAlt = 0.0
        for y in range(self.mapHeight):
            for x in range(self.mapWidth):
                plot = self.map[self.GetIndex(x,y)]
                if plot > maxAlt:
                    maxAlt = plot
                if plot < minAlt:
                    minAlt = plot
        #normalize map so that all altitudes are between 1 and 0
        #first add minAlt to all values if necessary
        if minAlt < 0.0:
            for y in range(self.mapHeight):
                for x in range(self.mapWidth):
                    self.map[self.GetIndex(x,y)] -= minAlt
        #add minAlt to maxAlt also before scaling entire map
        maxAlt -= minAlt
        scaler = 1.0/maxAlt
        for y in range(self.mapHeight):
            for x in range(self.mapWidth):
                self.map[self.GetIndex(x,y)] = self.map[self.GetIndex(x,y)] * scaler               
        return
    def GenerateFinalPlotTypes(self):
        self.OCEAN = 0
        self.LAND = 1
        self.HILLS = 2
        self.PEAK = 3

        #create height difference map to allow for tuning
        diffMap = array('d')
        for i in range(0,self.mapHeight*self.mapWidth):
            diffMap.append(0.0)
        for y in range(self.mapHeight):
            for x in range(self.mapWidth):
                i = self.GetIndex(x,y)
                myAlt = self.map[i]
                if myAlt > self.seaLevel:
                    minAlt = self.GetLowestNeighbor(x,y)
                    diffMap[i] = myAlt - minAlt
        peakHeight = FindValueFromPercent(diffMap,mc.PeakPercent,0.001,True)
        hillHeight = FindValueFromPercent(diffMap,mc.HillPercent,0.001,True)
##        #as self.chunkSize gets larger , the heightmap 
##        #tends to get smoother. This needs to be accounted for to get the
##        #desired number of hills/peaks
##        if self.chunkSize == 16:
##            hillHeight = .05 
##            peakHeight = .093
##        elif self.chunkSize == 8:
##            hillHeight = .055 
##            peakHeight = .11
##        elif self.chunkSize == 4:
##            hillHeight = .065 
##            peakHeight = .13
##        #the following two cases are untested
##        elif self.chunkSize == 32:
##            hillHeight = .045 
##            peakHeight = .089
##        elif self.chunkSize == 2:
##            hillHeight = .08 
##            peakHeight = .15
##        else:
##            raise ValueError,"Map size is either too small or too large."
        self.plotMap = array('i')
        #initialize map with 0CEAN
        for i in range(0,self.mapHeight*self.mapWidth):
            self.plotMap.append(self.OCEAN)
        for y in range(self.mapHeight):
            for x in range(self.mapWidth):
                i = self.GetIndex(x,y)
                myAlt = self.map[i]
                if myAlt > self.seaLevel:
                    minAlt = self.GetLowestNeighbor(x,y)
                    if minAlt < myAlt-peakHeight:
                        self.plotMap[i] = self.PEAK
                    elif minAlt < myAlt-hillHeight:
                        self.plotMap[i] = self.HILLS
                    else:
                        self.plotMap[i] = self.LAND
        
        return
    def GetLowestNeighbor(self,x,y):
        minAlt = 1.0
        for yy in range(y-1,y+1):
            for xx in range(x-1,x+1):
                i = self.GetIndex(xx,yy)
                myAlt = self.map[i]
                if myAlt < self.seaLevel: #seaLevel is minimum consideration
                    myAlt = self.seaLevel
                if(minAlt > myAlt):
                    minAlt = myAlt
        return minAlt
    def FillInLakes(self):
        #smaller lakes need to be filled in for now. The river system will
        #most likely recreate them later due to drainage calculation
        #according to certain rules. This makes the lakes look much better
        #and more sensible.
        am = Areamap(self.mapWidth,self.mapHeight)
        am.defineAreas(False)
##        am.PrintAreaMap()
        oceanID = am.getOceanID()
        for i in range(self.mapHeight*self.mapWidth):
            if self.plotMap[i] == self.OCEAN and am.areaMap[i] != oceanID:
                #check the size of this body of water, if too small,
                #change to land
                for a in am.areaList:
                    if a.ID == am.areaMap[i] and a.size < 35:
                        self.plotMap[i] = self.LAND
        
        return
#global access
print "initializing hm"
hm = Heightmap()

##This function is a general purpose value tuner. It finds a value that will be greater
##than or less than the desired percent of a whole map within a given tolerance. Map values
##should be between 0 and 1.
def FindValueFromPercent(mmap,percent,tolerance,greaterThan):
    inTolerance = False
    #to speed things up a little, lets take some time to find the middle value
    #in the dataset and use that to begin our search
    minV = 100.0
    maxV = 0.0
    for i in range(hm.mapHeight*hm.mapWidth):
        if minV > mmap[i]:
            minV = mmap[i]
        if maxV < mmap[i]:
            maxV = mmap[i]
    mid = (maxV - minV)/2.0 + minV
##    print "starting threshold = "
##    print mid
##    print "desired percent",percent
    threshold = mid
    thresholdChange = mid
    iterations = 0
    while not inTolerance:
        iterations += 1
        if(iterations > 500):
            print "can't find value within tolerance, end value = "
            print threshold, thresholdChange
            break #close enough
        matchCount = 0
##        print "threshold",threshold
        for i in range(hm.mapHeight*hm.mapWidth):
            if mmap[i] != 0.0:
                if greaterThan == True:
                    if(mmap[i] > threshold):
                        matchCount += 1
                else:
                    if(mmap[i] < threshold):  
                        matchCount += 1
##        print "matchCount",matchCount
        currentPercent = float(matchCount)/float(hm.mapHeight*hm.mapWidth)
##        print "currentPercent",currentPercent
        if currentPercent < percent + tolerance and \
           currentPercent > percent - tolerance:
            inTolerance = True
        elif greaterThan == True:
            if currentPercent < percent - tolerance:
                threshold -= thresholdChange
            else:
                threshold += thresholdChange
        else:
            if currentPercent > percent + tolerance:
                threshold -= thresholdChange
            else:
                threshold += thresholdChange
        if not inTolerance:
#                print "sealevel = " + str(self.seaLevel), "change = " + str(seaLevelChange)
            thresholdChange = thresholdChange/2.0

        #at this point value should be in tolerance or close to it
    return threshold
##############################################################################
## Custom Area tracker allows for considering coast as land to find
## 'New World' type continents that cannot be reached with galleys.
## Also used for filling in lakes since the required code is similar
##############################################################################
class Areamap :
    def __init__(self,width,height):
        self.mapWidth = width
        self.mapHeight = height
        self.areaMap = array('i')
        #initialize map with zeros
        for i in range(0,self.mapHeight*self.mapWidth):
            self.areaMap.append(0)
        return
    def defineAreas(self,coastIsLand):
        #coastIsLand = True means that we are trying to find continents that
        #are not connected by coasts to the main landmasses, allowing us to
        #find continents suitable as a 'New World'. Otherwise, we
        #are just looking to fill in lakes and coast needs to be considered
        #water in that case
#        self.areaSizes = array('i')
##        starttime = time.clock()
        self.areaList = list()
        areaID = 0
        #make sure map is erased in case it is used multiple times
        for i in range(0,self.mapHeight*self.mapWidth):
            self.areaMap[i] = 0
#        for i in range(0,1):
        for i in range(0,self.mapHeight*self.mapWidth):
            if self.areaMap[i] == 0: #not assigned to an area yet
                areaID += 1
                areaSize,isWater = self.fillArea(i,areaID,coastIsLand)
                area = Area(areaID,areaSize,isWater)
                self.areaList.append(area)

##        endtime = time.clock()
##        elapsed = endtime - starttime
##        print "defineAreas time ="
##        print elapsed
##        print

        return

    def isWater(self,x,y,coastIsLand):
        #coastIsLand = True means that we are trying to find continents that
        #are not connected by coasts to the main landmasses, allowing us to
        #find continents suitable as a 'New World'. Otherwise, we
        #are just looking to fill in lakes and coast needs to be considered
        #water in that case
        ii = self.getIndex(x,y)
        if ii == -1:
            return False
        if coastIsLand:
            if hm.plotMap[ii] == hm.OCEAN and terr.terrainMap[ii] != terr.COAST:
                return True
            else:
                return False
        else:
            if hm.plotMap[ii] == hm.OCEAN:
                return True
            else:
                return False
            
        return False
    def getAreaByID(self,areaID):
        for i in range(len(self.areaList)):
            if self.areaList[i].ID == areaID:
                return self.areaList[i]
        return None
    def getOceanID(self):
#        self.areaList.sort(key=operator.attrgetter('size'),reverse=True)
        self.areaList.sort(lambda x,y:cmp(x.size,y.size))
        self.areaList.reverse()
        for a in self.areaList:
            if a.water == True:
                return a.ID
            
    def getContinentCenter(self,ID):
        #first find center in x direction
        changes = list()
        yMin = hm.mapHeight
        yMax = -1
        meridianOverlap = False
        onContinent = False
        for x in range(hm.mapWidth):
            continentFoundThisPass = False
            for y in range(hm.mapHeight):
                i = self.getIndex(x,y)
                if self.areaMap[i] == ID:
                    continentFoundThisPass = True
                    if y < yMin:
                        yMin = y
                    elif y > yMax:
                        yMax = y
            if x == 0 and continentFoundThisPass:
                meridianOverlap = True
                onContinent = True
            if onContinent and not continentFoundThisPass:
                changes.append(x)
                onContinent = False
            elif not onContinent and continentFoundThisPass:
                changes.append(x)
                onContinent = True
        changes.sort()
        xCenter = -1
        if len(changes) == 0: #continent is continuous
            xCenter = -1
        elif len(changes) == 1:#continent extends to map edge
            if meridianOverlap:
                xCenter = changes[0]/2
            else:
                xCenter = (hm.mapWidth - changes[0])/2 + changes[0]
        else:
            if meridianOverlap:
                xCenter = ((changes[1] - changes[0])/2 + changes[0] + (hm.mapWidth/2)) % hm.mapWidth
            else:
                xCenter = (changes[1] - changes[0])/2 + changes[0]
        yCenter = (yMax - yMin)/2 + yMin
        center = xCenter,yCenter
        return center    

    def isPangea(self):
##        starttime = time.clock()
        continentList = list()
        for a in self.areaList:
            if a.water == False:
                continentList.append(a)

        totalLand = 0             
        for c in continentList:
            totalLand += c.size
            
        #sort all the continents by size, largest first
        continentList.sort(lambda x,y:cmp(x.size,y.size))
        continentList.reverse()
        biggestSize = continentList[0].size
        if 0.70 < float(biggestSize)/float(totalLand):
##            endtime = time.clock()
##            elapsed = endtime - starttime
##            print "isPangea time = %(t)s" % {"t":str(elapsed)}
            return True
##        endtime = time.clock()
##        elapsed = endtime - starttime
##        print "isPangea time = "
##        print elapsed
##        print
        return False
    def getMeteorStrike(self):
##        starttime = time.clock()
        continentList = list()
        for a in self.areaList:
            if a.water == False:
                continentList.append(a)
            
        #sort all the continents by size, largest first
        continentList.sort(lambda x,y:cmp(x.size,y.size))
        continentList.reverse()
        biggestContinentID = continentList[0].ID
        
        chokeList = list()
        for y in range(hm.mapHeight):
            for x in range(hm.mapWidth):
                i = self.getIndex(x,y)
                if self.areaMap[i] == biggestContinentID:
                    if self.isChokePoint(x,y):
                        ap = AreaPlot(x,y)
                        chokeList.append(ap)
        #calculate distances to center
        center = self.getContinentCenter(biggestContinentID)
        xCenter,yCenter = center
        for n in range(len(chokeList)):
            distance = self.getDistance(chokeList[n].x,chokeList[n].y,xCenter,yCenter)
            chokeList[n].avgDistance = distance
            
        #sort plotList for most avg distance and chokeList for least
        #average distance
        chokeList.sort(lambda x,y:cmp(x.avgDistance,y.avgDistance))

        if len(chokeList) == 0:#return bad value if no chokepoints
##            endtime = time.clock()
##            elapsed = endtime - starttime
##            print "getMeteorStrike time = "
##            print elapsed
##            print
            return -1,-1

##        endtime = time.clock()
##        elapsed = endtime - starttime
##        print "getMeteorStrike time = "
##        print elapsed
##        print
        
        return chokeList[0].x,chokeList[0].y
                
    def isChokePoint(self,x,y):
        circlePoints = self.getCirclePoints(x,y,4)
        waterOpposite = False
        landOpposite = False
        for cp in circlePoints:
            if self.isWater(cp.x,cp.y,True):
                #Find opposite
                ox = x + (x - cp.x)
                oy = y + (y - cp.y)
                if self.isWater(ox,oy,True):
                    waterOpposite = True
            else:
                #Find opposite
                ox = x + (x - cp.x)
                oy = y + (y - cp.y)
                if not self.isWater(ox,oy,True):
                    landOpposite = True
        if landOpposite and waterOpposite:
            return True
        return False
    def getDistance(self,x,y,dx,dy):
        xx = x - dx
        if abs(xx) > hm.mapWidth/2:
            xx = hm.mapWidth - abs(xx)
            
        distance = max(abs(xx),abs(y - dy))
        return distance
        
    def getNewWorldID(self):
        nID = 0
        continentList = list()
        for a in self.areaList:
            if a.water == False:
                continentList.append(a)

        totalLand = 0             
        for c in continentList:
            totalLand += c.size
            
        print totalLand

        #sort all the continents by size, largest first
#        continentList.sort(key=operator.attrgetter('size'),reverse=True)
        continentList.sort(lambda x,y:cmp(x.size,y.size))
        continentList.reverse()
        
        print ''
        print "All continents"
        print self.PrintList(continentList)

        #now remove a percentage of the landmass to be considered 'Old World'
        oldWorldSize = 0
        #biggest continent is automatically 'Old World'
        oldWorldSize += continentList[0].size
        del continentList[0]

        #get the next largest continent and temporarily remove from list
        #add it back later and is automatically 'New World'
        biggestNewWorld = continentList[0]
        del continentList[0]
        
        #sort list by ID rather than size to make things
        #interesting and possibly bigger new worlds
#        continentList.sort(key=operator.attrgetter('ID'),reverse=True)
        continentList.sort(lambda x,y:cmp(x.ID,y.ID))
        continentList.reverse()
        
        for n in range(len(continentList)):
            oldWorldSize += continentList[0].size
            del continentList[0]
            if float(oldWorldSize)/float(totalLand) > 0.60:
                break

        #add back the biggestNewWorld continent
        continentList.append(biggestNewWorld)
        
        #what remains in the list will be considered 'New World'
        print ''
        print "New World Continents"
        print self.PrintList(continentList)

        #get ID for the next continent, we will use this ID for 'New World'
        #designation
        nID = continentList[0].ID
        del continentList[0] #delete to avoid unnecessary overwrite

        #now change all the remaining continents to also have nID as their ID
        for i in range(self.mapHeight*self.mapWidth):
            for c in continentList:
                if c.ID == self.areaMap[i]:
                    self.areaMap[i] = nID
 
        return nID
            
    def getIndex(self,x,y):
        #wrap in X direction only.  
        dx = x
        if y >= self.mapHeight or y < 0:
            return -1
        
        if x >= self.mapWidth:
            dx = x % self.mapWidth
        elif x < 0:
            dx = x % self.mapWidth
            
        index = y*self.mapWidth + dx
        return index
    
    def fillArea(self,index,areaID,coastIsLand):
        #first divide index into x and y
        y = index/self.mapWidth
        x = index%self.mapWidth
        #We check 8 neigbors for land,but 4 for water. This is because
        #the game connects land squares diagonally across water, but
        #water squares are not passable diagonally across land
        self.segStack = list()
        self.size = 0
        isAreaWater = self.isWater(x,y,coastIsLand)
        #place seed on stack for both directions
        seg = LineSegment(y,x,x,1)
        self.segStack.append(seg) 
        seg = LineSegment(y+1,x,x,-1)
        self.segStack.append(seg) 
        while(len(self.segStack) > 0):
            seg = self.segStack.pop()
            self.scanAndFillLine(seg,areaID,isAreaWater,coastIsLand)
##            if (seg.y < 8 and seg.y > 4) or (seg.y < 70 and seg.y > 64):
##            if (areaID == 4
##                PrintPlotMap(hm)
##                self.PrintAreaMap()
        
        return self.size,self.isWater(x,y,coastIsLand)
    def scanAndFillLine(self,seg,areaID,isWater,coastIsLand):
        #check for y + dy being off map
        i = self.getIndex(seg.xLeft,seg.y + seg.dy)
        if i < 0:
##            print "scanLine off map ignoring",str(seg)
            return
        debugReport = False
##        if (seg.y < 8 and seg.y > 4) or (seg.y < 70 and seg.y > 64):
##        if (areaID == 4):
##            debugReport = True
        #for land tiles we must look one past the x extents to include
        #8-connected neighbors
        if isWater:
            landOffset = 0
        else:
            landOffset = 1
        lineFound = False
        #first scan and fill any left overhang
        if debugReport:
            print ""
            print "areaID = %(a)4d" % {"a":areaID}
            print "isWater = %(w)2d, landOffset = %(l)2d" % {"w":isWater,"l":landOffset} 
            print str(seg)
            print "Going left"
        for xLeftExtreme in range(seg.xLeft - landOffset,0 - (self.mapWidth*20),-1):
            i = self.getIndex(xLeftExtreme,seg.y + seg.dy)
            if debugReport:
                print "xLeftExtreme = %(xl)4d" % {'xl':xLeftExtreme}
            if self.areaMap[i] == 0 and isWater == self.isWater(xLeftExtreme,seg.y + seg.dy,coastIsLand):
                self.areaMap[i] = areaID
                self.size += 1
                lineFound = True
            else:
                #if no line was found, then xLeftExtreme is fine, but if
                #a line was found going left, then we need to increment
                #xLeftExtreme to represent the inclusive end of the line
                if lineFound:
                    xLeftExtreme += 1
                break
        if debugReport:
            print "xLeftExtreme finally = %(xl)4d" % {'xl':xLeftExtreme}
            print "Going Right"
        #now scan right to find extreme right, place each found segment on stack
#        xRightExtreme = seg.xLeft - landOffset #needed sometimes? one time it was not initialized before use.
        xRightExtreme = seg.xLeft #needed sometimes? one time it was not initialized before use.
        for xRightExtreme in range(seg.xLeft,self.mapWidth*20,1):
            if debugReport:            
                print "xRightExtreme = %(xr)4d" % {'xr':xRightExtreme}
            i = self.getIndex(xRightExtreme,seg.y + seg.dy)
            if self.areaMap[i] == 0 and isWater == self.isWater(xRightExtreme,seg.y + seg.dy,coastIsLand):
                self.areaMap[i] = areaID
                self.size += 1
                if lineFound == False:
                    lineFound = True
                    xLeftExtreme = xRightExtreme #starting new line
                    if debugReport:
                        print "starting new line at xLeftExtreme= %(xl)4d" % {'xl':xLeftExtreme}
            elif lineFound == True: #found the right end of a line segment!                
                lineFound = False
                #put same direction on stack
                newSeg = LineSegment(seg.y + seg.dy,xLeftExtreme,xRightExtreme - 1,seg.dy)
                self.segStack.append(newSeg)
                if debugReport:
                    print "same direction to stack",str(newSeg)
                #determine if we must put reverse direction on stack
                if xLeftExtreme < seg.xLeft or xRightExtreme >= seg.xRight:
                    #out of shadow so put reverse direction on stack also
                    newSeg = LineSegment(seg.y + seg.dy,xLeftExtreme,xRightExtreme - 1,-seg.dy)
                    self.segStack.append(newSeg)
                    if debugReport:
                        print "opposite direction to stack",str(newSeg)
                if xRightExtreme >= seg.xRight + landOffset:
                    if debugReport:
                        print "finished with line"
                    break; #past the end of the parent line and this line ends
            elif lineFound == False and xRightExtreme >= seg.xRight + landOffset:
                if debugReport:
                    print "no additional lines found"
                break; #past the end of the parent line and no line found
            else:
                continue #keep looking for more line segments
        if lineFound == True: #still a line needing to be put on stack
            if debugReport:
                print "still needing to stack some segs"
            lineFound = False
            #put same direction on stack
            newSeg = LineSegment(seg.y + seg.dy,xLeftExtreme,xRightExtreme - 1,seg.dy)
            self.segStack.append(newSeg)
            if debugReport:
                print str(newSeg)
            #determine if we must put reverse direction on stack
            if xLeftExtreme < seg.xLeft or xRightExtreme - 1 > seg.xRight:
                #out of shadow so put reverse direction on stack also
                newSeg = LineSegment(seg.y + seg.dy,xLeftExtreme,xRightExtreme - 1,-seg.dy)
                self.segStack.append(newSeg)
                if debugReport:
                    print str(newSeg)
        
        return
    #for debugging
    def PrintAreaMap(self):
        
        print "Area Map"
        for y in range(hm.mapHeight - 1,-1,-1):
            lineString = ""
            for x in range(hm.mapWidth):
                mapLoc = self.areaMap[hm.GetIndex(x,y)]
                if mapLoc + 34 > 127:
                    mapLoc = 127 - 34
                lineString += chr(mapLoc + 34)
            lineString += "-" + str(y)
            print lineString
        oid = self.getOceanID()
        if oid == None or oid + 34 > 255:
            print "Ocean ID is unknown"
        else:
            print "Ocean ID is %(oid)4d or %(c)s" % {'oid':oid,'c':chr(oid + 34)}
        lineString = " "
        print lineString

        return
    def PrintList(self,s):
        for a in s:
            char = chr(a.ID + 34)
            lineString = str(a) + ' ' + char
            print lineString
            
    def getCirclePoints(self,xCenter,yCenter,radius):
        circlePointList = list()
        x = 0
        y = radius
        p = 1 - radius

        self.addCirclePoints(xCenter,yCenter,x,y,circlePointList)

        while (x < y):
            x += 1
            if p < 0:
                p += 2*x + 1
            else:
                y -= 1
                p += 2*(x - y) + 1
            self.addCirclePoints(xCenter,yCenter,x,y,circlePointList)
            
        return circlePointList
    
    def addCirclePoints(self,xCenter,yCenter,x,y,circlePointList):
        circlePointList.append(CirclePoint(xCenter + x,yCenter + y))
        circlePointList.append(CirclePoint(xCenter - x,yCenter + y))
        circlePointList.append(CirclePoint(xCenter + x,yCenter - y))
        circlePointList.append(CirclePoint(xCenter - x,yCenter - y))
        circlePointList.append(CirclePoint(xCenter + y,yCenter + x))
        circlePointList.append(CirclePoint(xCenter - y,yCenter + x))
        circlePointList.append(CirclePoint(xCenter + y,yCenter - x))
        circlePointList.append(CirclePoint(xCenter - y,yCenter - x))
        return
class CirclePoint :
    def __init__(self,x,y):
        self.x = x
        self.y = y

class LineSegment :
    def __init__(self,y,xLeft,xRight,dy):
        self.y = y
        self.xLeft = xLeft
        self.xRight = xRight
        self.dy = dy
    def __str__ (self):
        string = "y = %(y)3d, xLeft = %(xl)3d, xRight = %(xr)3d, dy = %(dy)2d" % \
        {'y':self.y,'xl':self.xLeft,'xr':self.xRight,'dy':self.dy}
        return string
                       
class Area :
    def __init__(self,iD,size,water):
        self.ID = iD
        self.size = size
        self.water = water

    def __str__(self):
        string = "{ID = %(i)4d, size = %(s)4d, water = %(w)1d}" % \
        {'i':self.ID,'s':self.size,'w':self.water}
        return string
class AreaPlot :
    def __init__(self,x,y):
        self.x = x
        self.y = y
        self.avgDistance = -1
                       
##########################################################################
##Global Normalized map function
def NormalizeMap(Width,Height,fMap):
    #find highest and lowest points
    maxAlt = 0.0
    minAlt = 0.0
    for y in range(Height):
        for x in range(Width):
            plot = fMap[hm.GetIndex(x,y)]
            if plot > maxAlt:
                maxAlt = plot
            if plot < minAlt:
                minAlt = plot
    #normalize map so that all altitudes are between 1 and 0
    #first add minAlt to all values if necessary
    if minAlt < 0.0:
        for y in range(Height):
            for x in range(Width):
                fMap[hm.GetIndex(x,y)] -= minAlt
    #add minAlt to maxAlt also before scaling entire map
    maxAlt -= minAlt
    scaler = 1.0/maxAlt
    for y in range(Height):
        for x in range(Width):
            fMap[hm.GetIndex(x,y)] = fMap[hm.GetIndex(x,y)] * scaler              
    return

##############################################################################
## Terrain generation
##############################################################################
'''
This class is used by the TemperatureMap and RainfallMap classes to govern the
general behavior of winds. Prevailing winds are the most important factor in
distributing temperature and rainfall, but they tend to fluctuate a great deal
depending on the time of year and the surrounding geology. Therefore, prevailing
surface winds are simulated in this script by a right-triangle shaped area
with it's base as the source of distribution. It can be thought of as a
'shotgun blast' eminating from the source map square.
^     7
|    /
|   /
|  /
| /
S----------->
The influence of the source square decreases as the distance from it increases
'''
class WindZones :
    def __init__(self,mapHeight,topLat,botLat):
        self.NPOLAR = 0
        self.NTEMPERATE = 1
        self.NEQUATOR = 2
        self.SEQUATOR = 3
        self.STEMPERATE = 4
        self.SPOLAR = 5
        self.NOZONE = 99
        self.mapHeight = mapHeight
        self.topLat = topLat
        self.botLat = botLat
    def GetZone(self,y):
        if y < 0 or y >= self.mapHeight:
            return self.NOZONE
        lat = self.GetLatitude(y)
        if lat > 60:
            return self.NPOLAR
        elif lat > 25:
            return self.NTEMPERATE
        elif lat > 0:
            return self.NEQUATOR
        elif lat > -25:
            return self.SEQUATOR
        elif lat > -60:
            return self.STEMPERATE
        else:
            return self.SPOLAR
        return
    def GetZoneName(self,zone):
        if zone == self.NPOLAR:
            return "NPOLAR"
        elif zone == self.NTEMPERATE:
            return "NTEMPERATE"
        elif zone == self.NEQUATOR:
            return "NEQUATOR"
        elif zone == self.SEQUATOR:
            return "SEQUATOR"
        elif zone == self.STEMPERATE:
            return "STEMPERATE"
        else:
            return "SPOLAR"
        return
    def GetYFromZone(self,zone,bTop):
        if bTop:
            for y in range(hm.mapHeight - 1,-1,-1):
                if zone == self.GetZone(y):
                    return y
        else:
            for y in range(hm.mapHeight):
                if zone == self.GetZone(y):
                    return y
        return 
    def GetBlastBoxHeight(self,y):
        zone = self.GetZone(y)
        we,ns = self.GetWindDirections(y)
        if ns < 0:
            maxY = self.GetYFromZone(zone,True) - 1#zone *before* this is max
        else:
            maxY = self.GetYFromZone(zone,False) + 1 #zone *after* this is max
        height = maxY - y 
        return height
    def GetZoneSize(self):
        latitudeRange = self.topLat - self.botLat
        degreesPerDY = float(latitudeRange)/float(self.mapHeight)
        size = 30.0/degreesPerDY
        return size
    def GetLatitude(self,y):
        latitudeRange = self.topLat - self.botLat
        degreesPerDY = float(latitudeRange)/float(self.mapHeight)
        latitude = (self.topLat - (int(round(float(y)* degreesPerDY)))) * -1
        return latitude
    def GetWindDirections(self,y):
        z = self.GetZone(y)
        #get x,y directions
        return self.GetWindDirectionsInZone(z)
    def GetWindDirectionsInZone(self,z):
        #get x,y directions
        if z == self.NPOLAR:
            return (-1,-1)
        elif z == self.NTEMPERATE:
            return (1,1)
        elif z == self.NEQUATOR:
            return (-1,-1)
        elif z == self.SEQUATOR:
            return (-1,1)
        elif z == self.STEMPERATE:
            return (1,-1)
        elif z == self.SPOLAR:
            return (-1,1)
        return (0,0)
'''
This class generates and provides access to a temperature map of the world.
Temperatures range from 0.0(coldest) to 1.0(hottest)
'''
class TemperatureMap :
    def __init__(self):
        #To provide global access without allocating alot of resources for
        #nothing, object initializer must be empty
        return
    def GenerateTempMap(self,topLatitude,bottomLatitude):
        self.tempMap = array('d')
        self.sunMap = array('d')
        #initialize map with zeros
        for i in range(0,hm.mapHeight*hm.mapWidth):
            self.tempMap.append(0.0)
            self.sunMap.append(0.0)

        self.wz = WindZones(hm.mapHeight,topLatitude,bottomLatitude)
        for y in range(hm.mapHeight):
            for x in range(hm.mapWidth):
                self.SetInitialTemperature(x,y)
        NormalizeMap(hm.mapWidth,hm.mapHeight,self.sunMap)
        NormalizeMap(hm.mapWidth,hm.mapHeight,self.tempMap)
        for z in range(self.wz.NPOLAR,self.wz.SPOLAR + 1):
            we,ns = self.wz.GetWindDirectionsInZone(z)
            if ns < 0:
                yStart = self.wz.GetYFromZone(z,False)
                yStop = self.wz.GetYFromZone(z,True) + ns
            else:
                yStart = self.wz.GetYFromZone(z,True)
                yStop = self.wz.GetYFromZone(z,False) + ns
            for y in range(yStart,yStop,ns):
                #flip we for the x loop so that it goes opposite of wind
                #direction
                if we > 0:
                    xStart = hm.mapWidth - 1
                    xStop = -1
                else:
                    xStart = 0
                    xStop = hm.mapWidth
                for x in range(xStart,xStop,(-1)*we):
                    self.DistributeTemp(x,y,we,ns)
        for y in range(hm.mapHeight):
            for x in range(hm.mapWidth):
                i = hm.GetIndex(x,y)
                if y == 0 and self.tempMap[i] > .6:
                    print "***************************************"
                    print "!!!! " + str(x) +',' + str(y) + " = " + str(self.tempMap[i])
                    print "***************************************"
#        NormalizeMap(hm.mapWidth,hm.mapHeight,self.tempMap)
        return
    def SetInitialTemperature(self,x,y):
        #this procedure decides how much heat is present at a location
        #before any heat transfer process takes place.
        oceanPoleTemp = .10
        lat = abs(float(self.wz.GetLatitude(y)))
        
        i = hm.GetIndex(x,y)
        if hm.plotMap[i] == hm.OCEAN:
            oceanTempRange = 1.0 - oceanPoleTemp
            latRange = 90.0 #always
            tempPerLatChange = oceanTempRange/latRange
            temp = (latRange - lat) * tempPerLatChange + oceanPoleTemp
            self.sunMap[i] = temp
            self.tempMap[i] = temp
        else:
            latRange = 90.0
            tempPerLatChange = 1.0/latRange
            temp = (latRange - lat) * tempPerLatChange
            #modify temperature by altitude, to reflect that higher
            #altitudes have diminished capacity to retain heat 
            altitude = hm.map[i]
            altitudeRange = 1.0 - hm.seaLevel
            altitudeModifier = (altitude - hm.seaLevel)* 1/altitudeRange
            temp = temp * (1.0 - altitudeModifier)
            self.sunMap[i] = temp
            self.tempMap[i] = temp
            
        return
    def DistributeTemp(self,x,y,we,ns):
        bDebug = False
        xStop = x + (hm.mapWidth/6 * we)
        i = hm.GetIndex(x,y)
        xWeight = 0.40
        yWeight = 0.35
        if self.wz.GetZone(y) == self.wz.NPOLAR:
#            bDebug = True
            pass
        for xx in range(x,xStop,we):
            i = hm.GetIndex(xx,y)
            xBack = hm.GetIndex(xx - we,y)
            sourceTemp = self.tempMap[xBack]
            if bDebug:
                print xx,y
                print self.tempMap[i], sourceTemp
            self.tempMap[i] = self.tempMap[i] * (1.0 - xWeight) + sourceTemp  * xWeight
            if bDebug:
                print self.tempMap[i]
            if self.wz.GetZone(y) == self.wz.GetZone(y - ns):
                yBack = hm.GetIndex(xx,y - ns)
                sourceTemp = self.tempMap[yBack]
                if bDebug:
                    print self.tempMap[i],sourceTemp
                self.tempMap[i] = self.tempMap[i] * (1.0 - yWeight) + sourceTemp * yWeight
                if bDebug:
                    print self.tempMap[i]
            xWeight = xWeight*xWeight
            yWeight = yWeight*yWeight
            if bDebug:
                print ""
                
        return
#global access
tm = TemperatureMap()

class RainfallMap :
    def __init__(self):
        #To provide global access without allocating alot of resources for
        #nothing, object initializer must be empty
        return
    def GenerateRainMap(self,topLatitude,bottomLatitude):
        print "generating rain map"
        self.rainMap = array('d')
        smoothMap = array('d')
        self.distHeight = hm.mapWidth/4 #yes, mapWIDTH /4 the dist maps are square
        self.distWidth = hm.mapWidth/4
        print "distWidth-Height %(x)02d,%(y)02d" % {'x' : self.distWidth, 'y' : self.distHeight}
        rainDumped = array('d')#records rainfall from one square overwriting each time
        rainPotential = array('d')
        #initialize map with zeros or tempMap value if ocean
        for i in range(0,hm.mapHeight*hm.mapWidth):
            smoothMap.append(0.0)
            if hm.plotMap[i] == hm.OCEAN:
                self.rainMap.append(tm.tempMap[i])
            else:
                self.rainMap.append(0.1)
        #initialize local distribution maps
        for i in range(0,self.distHeight * self.distWidth):
            rainDumped.append(0.0)
            rainPotential.append(0.0)
        #step through each windzone
        self.wz = WindZones(hm.mapHeight,topLatitude,bottomLatitude)
        for z in range(self.wz.NPOLAR,self.wz.SPOLAR + 1):
            we,ns = self.wz.GetWindDirectionsInZone(z)
            self.printDetails = False
            print we,ns,PrintWindDirection(we,ns)
            if ns > 0:
                yStart = self.wz.GetYFromZone(z,False)
                yStop = self.wz.GetYFromZone(z,True) + ns
            else:
                yStart = self.wz.GetYFromZone(z,True)
                yStop = self.wz.GetYFromZone(z,False) + ns
            print "Looping in Y from ",yStart,"to",yStop
            for y in range(yStart,yStop,ns):
                if we < 0:
                    xStart = hm.mapWidth - 1
                    xStop = -1
                else:
                    xStart = 0
                    xStop = hm.mapWidth
                for x in range(xStart,xStop,we):
                    i = hm.GetIndex(x,y)
                    #print x,y
                    if(hm.plotMap[i] == hm.OCEAN):
                        self.DistributeRain(rainDumped,rainPotential,x,y,yStop,we,ns)
                        self.printDetails = False
        #Add some smoothing and noise to the rain map to avoid harsh
        #looking artifacts
        for y in range(hm.mapHeight):
            for x in range(hm.mapWidth):
                i = hm.GetIndex(x,y)
                avg = 0.0
                for yy in range(y-1,y+2):
                    for xx in range(x-1,x+2):
                        ii = hm.GetIndex(x,y)
                        avg += self.rainMap[ii]
                avg = avg/9.0
                smoothMap[i] = (self.rainMap[i] + (avg * mc.RainSmoothingFactor) + (PWRand.random()* mc.RainNoise * self.rainMap[i]))/(1 + mc.RainSmoothingFactor + (mc.RainNoise * self.rainMap[i]))
        for i in range(0,hm.mapHeight*hm.mapWidth):
            self.rainMap[i] = smoothMap[i]
                            
        #We dont care about ocean rainfall, so set those to zero before
        #normalization
        for y in range(hm.mapHeight):
            for x in range(hm.mapWidth):
                i = hm.GetIndex(x,y)
                if self.rainMap[i] < 0.0:
                    raise ValueError,"negative values detected"
                if hm.plotMap[i] == hm.OCEAN:
                    self.rainMap[i] = 0.0
     
        NormalizeMap(hm.mapWidth,hm.mapHeight,self.rainMap)
        
        return
    def DistributeRain(self,rd,rp,x,y,oldYStop,we,ns):
        #set up ranges and offsets
        if ns < 0:
            yStart = self.distHeight - 1
            yStop = yStart - (y - oldYStop)
            offsetY = y - yStart 
        else:
            yStart = 0
            yStop = oldYStop - y
            offsetY = y
        if we > 0:
            xStart = 0
            xStop = self.distWidth 
            offsetX = x
        else:
            xStart = self.distWidth - 1
            xStop = -1
            offsetX = x - (self.distWidth - 1)
        
        if self.printDetails == True:
            print "yStart = %(ys)3d yStop = %(yp)3d" % {'ys' : yStart,'yp' : yStop}
            print "offsetY = %(off)3d" % {'off' : offsetY}
        #seed corner with tempMap(x,y)
        i = hm.GetIndex(x,y)
        ii = yStart * self.distWidth + xStart
        rp[ii] = tm.tempMap[i]
        lastRow = False
        for yy in range(yStart,yStop,ns):
            for xx in range(xStart,xStop,we):
                if yy == yStop - ns:
                    lastRow = True
                self.SplashRain(rd,rp,offsetX,offsetY,xx,yy,ns,we,lastRow)

        if self.printDetails == True:
            print "Potential"
            PrintDistMap(rp,self.distWidth,self.distHeight)    
            print "Dumped"
            PrintDistMap(rd,self.distWidth,self.distHeight)
            print "Copying these maps to %(x)2d , %(y)2d " % {'x' : x, 'y' : y} + PrintWindDirection(we,ns)
        for yy in range(yStart,yStop,ns):
            for xx in range(xStart,xStop,we):
                i = hm.GetIndex(xx + offsetX,yy + offsetY)
                ii = (yy * self.distWidth + xx)
                self.rainMap[i] += rd[ii]
                if self.printDetails == True:
                    print "Add %(xx)3d , %(yy)3d to %(offX)3d , %(offY)3d amount = %(am)0.8f" % \
                    {'xx':xx,'yy':yy,'offX':xx + offsetX,'offY':yy + offsetY,'am':rd[ii]}
                

        
                
        return
    def SplashRain(self,rd,rp,offsetX,offsetY,x,y,ns,we,lastRow):
        #first pull rain potential from the y direction and dump the excess there,
        #then push rain potential to the x direction and dump the excess here.
        #This mimicks the operation of spreading the rain in two (NE etc.)
        #directions simultaniously for each square
        xDamper = 0.120#These dampers diminish the amount of moisture loss coming 
        yDamper = 0.160#from temperature changes
        i = y * self.distWidth + x
        imap = hm.GetIndex(x + offsetX,y + offsetY)
        if self.printDetails == True:
            print "imap coords = %(x)03d , %(y)03d" % {'x':x + offsetX,'y':y + offsetY}
        yy = y - ns
        if(yy >= 0 and yy < self.distHeight): #dont access over the dist maps
            ii = yy * self.distWidth + x #ii = source address
            #first calculate temp difference. temp from 1.0 to 0 will dump all
            #the rain, from 0.0 to 1.0 will dump none. Dump the rain at the
            #source square, then subtract rain dumped from current rain
            #potential and pass on to destination
            iimap = hm.GetIndex(x + offsetX,yy + offsetY)
            if self.printDetails == True:
                print "iimap coords Y = %(x)03d , %(y)03d" % {'x':x + offsetX,'y':yy + offsetY}
 #               self.printDetails = False
            tempDifference = tm.tempMap[iimap] - tm.tempMap[imap]
            tempFactor = tempDifference * 0.5 + 0.5 #convert -1 to 1 range to 0 to 1
            tempFactor *= yDamper #diminish effect so rain doesn't all dump on the coast
            rainToDump = rp[ii] * 0.5 * tempFactor
            rainToPassOn = rp[ii] * 0.5 - rainToDump
            rd[ii] += rainToDump
            if((x == 0 and we > 0) or (x == self.distWidth - 1 and we < 0)):
                rp[i] = rainToPassOn #overwrite on first column
            else:
                rp[i] += rainToPassOn        
                    
        #now do essentially the same thing in the x direction except the
        #source and destination are switched. Also, as each square is initially
        #approached from the x direction first, we overwrite, rather than adding
        #to existing rain dump
        xx = x + we
        if(xx >= 0 and xx < self.distWidth):
            ii = y * self.distWidth + xx #ii = destination address
            iimap = hm.GetIndex(xx + offsetX,y + offsetY)
            if self.printDetails == True:
                print "iimap coords X = %(x)03d , %(y)03d" % {'x':xx + offsetX,'y':y + offsetY}
            tempDifference = tm.tempMap[imap] - tm.tempMap[iimap]
            tempFactor = tempDifference * 0.5 + 0.5 #convert -1 to 1 range to 0 to 1
            tempFactor *= xDamper #diminish effect so rain doesn't all dump on the coast
            rainToDump = rp[i] * 0.5 * tempFactor
            rainToPassOn = rp[i] * 0.5 - rainToDump
            if lastRow == True:
                rd[i] = rp[i] * xDamper #windzone boundary dump it all!
            else:
                rd[i] = rainToDump #overwriting old data, no +=
            rp[ii] = rainToPassOn #overwriting old data
        else:
            #overwrite rainToDump on last call in x direction
            rd[i] = 0.0
        return
    
##Global access
rm = RainfallMap()

class TerrainMap :
    def __init__(self):
        #To provide global access without allocating alot of resources for
        #nothing, object initializer must be empty
        return
    def GenerateTerrainMap(self):
# Rise of Mankind start 2.5
		#self.DESERT = 0
        #self.PLAINS = 1
        #self.ICE = 2
        #self.TUNDRA = 3
        #self.GRASS = 4
        #self.HILL = 5
        #self.COAST = 6
        #self.OCEAN = 7
        #self.PEAK = 8
        self.DESERT = 0
        self.PLAINS = 1
        self.ICE = 2
        self.TUNDRA = 3
        self.MARSH = 4
        self.GRASS = 5
        self.HILL = 6
        self.COAST = 7
        self.OCEAN = 8
        self.PEAK = 9
        
        self.terrainMap = array('i')
        #initialize terrainMap with OCEAN
        for i in range(0,hm.mapHeight*hm.mapWidth):
            self.terrainMap.append(self.OCEAN)

        #Find minimum rainfall on land
        minRain = 10.0
        for i in range(hm.mapWidth*hm.mapHeight):
            if hm.plotMap[i] != hm.OCEAN:
                if rm.rainMap[i] < minRain:
                    minRain = rm.rainMap[i]
                    
        self.desertThreshold = FindValueFromPercent(rm.rainMap,mc.DesertPercent,.0001,False)
        self.plainsThreshold = FindValueFromPercent(rm.rainMap,mc.PlainsPercent,.0001,False)
# Rise of Mankind 2.5		
        self.marshThreshold = FindValueFromPercent(rm.rainMap,mc.MarshPercent,.0001,False)
# Rise of Mankind 2.5		
        for y in range(hm.mapHeight):
            for x in range(hm.mapWidth):
                i = hm.GetIndex(x,y)
                if hm.plotMap[i] == hm.OCEAN:
                    for yy in range(y - 1,y + 2,1):
                        for xx in range(x - 1,x + 2,1):
                            ii = hm.GetIndex(xx,yy)
                            if hm.plotMap[ii] != hm.OCEAN:
                                self.terrainMap[i] = self.COAST

                #instead of harsh thresholds, allow a random deviation chance
                #based on how close to the threshold the rainfall is
                elif rm.rainMap[i] < self.desertThreshold:
                    if tm.tempMap[i] < mc.IceTemp:
                        self.terrainMap[i] = self.ICE
                    elif tm.tempMap[i] < mc.TundraTemp:
                        self.terrainMap[i] = self.TUNDRA
                    elif tm.tempMap[i] < mc.DesertLowTemp:
                        self.terrainMap[i] = self.PLAINS
                    else:
                        if rm.rainMap[i] < (PWRand.random() * (self.desertThreshold - minRain) + self.desertThreshold - minRain)/2.0 + minRain:
                            self.terrainMap[i] = self.DESERT
                        else:
                            self.terrainMap[i] = self.PLAINS
# Rise of Mankind 2.5
                elif rm.rainMap[i] < self.marshThreshold:
                    if tm.tempMap[i] < mc.IceTemp:
                        self.terrainMap[i] = self.ICE
                    elif tm.tempMap[i] < mc.TundraTemp:
                        self.terrainMap[i] = self.TUNDRA
                    elif tm.tempMap[i] < mc.MarshLowTemp:
                        self.terrainMap[i] = self.PLAINS
                    else:
                        if rm.rainMap[i] < (PWRand.random() * (self.marshThreshold - self.plainsThreshold) + self.marshThreshold - self.plainsThreshold)/2.0 + self.marshThreshold:
                            self.terrainMap[i] = self.MARSH
                        else:
                            self.terrainMap[i] = self.PLAINS
# Rise of Mankind 2.5
                elif rm.rainMap[i] < self.plainsThreshold:
                    if tm.tempMap[i] < mc.IceTemp:
                        self.terrainMap[i] = self.ICE
                    elif tm.tempMap[i] < mc.TundraTemp:
                        self.terrainMap[i] = self.TUNDRA
                    else:
                        if rm.rainMap[i] < ((PWRand.random() * (self.plainsThreshold - self.desertThreshold) + self.plainsThreshold - self.desertThreshold))/2.0 + self.desertThreshold: 
                            self.terrainMap[i] = self.PLAINS
                        else:
                            self.terrainMap[i] = self.GRASS
                else:
                    if tm.tempMap[i] < mc.IceTemp:
                        self.terrainMap[i] = self.ICE
                    elif tm.tempMap[i] < mc.TundraTemp:
                        self.terrainMap[i] = self.TUNDRA
                    else:
                        self.terrainMap[i] = self.GRASS
        return
    def generateContinentMap(self):
        self.cm = Areamap(hm.mapWidth,hm.mapHeight)
        self.cm.defineAreas(True)
        meteorThrown = False
        pangeaDetected = False
#        self.cm.PrintAreaMap()
        while not mc.AllowPangeas and self.cm.isPangea():
            pangeaDetected = True
            x,y = self.cm.getMeteorStrike()
            if x == -1:
                print "Can't break pangea with meteors. No more chokepoints found."
                break
            print "A meteor has struck the Earth at %(x)d, %(y)d!!" % {"x":x,"y":y}
            self.castMeteorUponTheEarth(x,y)
#            self.cm.PrintAreaMap()
            meteorThrown = True
            self.cm.defineAreas(True)
        if meteorThrown:
            print "The age of dinosours has come to a cataclysmic end."
        if mc.AllowPangeas:
            print "Pangeas are allowed on this map and will not be suppressed."
        elif pangeaDetected == False:
            print "No pangea detected on this map."
#        self.cm.PrintAreaMap()
        self.newWorldID = self.cm.getNewWorldID()
#        self.cm.PrintAreaMap()
        return
    def castMeteorUponTheEarth(self,x,y):
##        starttime = time.clock()
        radius = PWRand.randint(4,max(5,hm.mapWidth/16))
        circlePointList = self.cm.getCirclePoints(x,y,radius)
##        print "circlePointList"
##        print circlePointList
        circlePointList.sort(lambda n,m:cmp(n.y,m.y))
        for n in range(0,len(circlePointList),2):
            cy = circlePointList[n].y
            if circlePointList[n].x < circlePointList[n + 1].x:
                x1 = circlePointList[n].x
                x2 = circlePointList[n + 1].x
            else:
                x2 = circlePointList[n].x
                x1 = circlePointList[n + 1].x
            self.drawCraterLine(x1,x2,cy)
            
        for n in range(0,len(circlePointList),2):
            cy = circlePointList[n].y
            if circlePointList[n].x < circlePointList[n + 1].x:
                x1 = circlePointList[n].x
                x2 = circlePointList[n + 1].x
            else:
                x2 = circlePointList[n].x
                x1 = circlePointList[n + 1].x
            self.drawCraterCoastLine(x1,x2,cy)
##        endtime = time.clock()
##        elapsed = endtime - starttime
##        print "castMeteorUponTheEarth time = "
##        print elapsed
##        print
        return
    def drawCraterCoastLine(self,x1,x2,y):
        if y < 0 or y >= hm.mapHeight:
            return
        for x in range(x1,x2 + 1):
            if self.hasLandNeighbor(x,y):
                i = hm.GetIndex(x,y)
                self.terrainMap[i] = self.COAST                   
        return
    def drawCraterLine(self,x1,x2,y):
        if y < 0 or y >= hm.mapHeight:
            return
        for x in range(x1,x2 + 1):
            i = hm.GetIndex(x,y)
            self.terrainMap[i] = self.OCEAN
            hm.map[i] = hm.seaLevel - 0.01
            hm.plotMap[i] = hm.OCEAN
        return
    def hasLandNeighbor(self,x,y):
        #y cannot be outside of map so I'm not testing for it
        for yy in range(y - 1,y + 2):
            for xx in range(x - 1,x + 2):
                if yy == y and xx == x:
                    continue
                ii = hm.GetIndex(xx,yy)
                if self.terrainMap[ii] != self.COAST and self.terrainMap[ii] != self.OCEAN:
                    return True
        return False
##Global access
terr = TerrainMap()                    

#OK! now that directions N,S,E,W are important, we have to keep in mind that
#the map plots are ordered from 0,0 in the SOUTH west corner! NOT the northwest
#corner! That means that Y increases as you go north.
class RiverMap :
    def __init__(self):
        #To provide global access without allocating alot of resources for
        #nothing, object initializer must be empty
        return
    def GenerateRiverMap(self):
        self.L = 0 #also denotes a 'pit' or 'flat'
        self.N = 1
        self.S = 2
        self.E = 3
        self.W = 4
        self.O = 5 #used for ocean or land without a river

        #averageHeightMap, flowMap, averageRainfallMap and drainageMap are offset from the other maps such that
        #each element coincides with a four tile intersection on the game map
        self.averageHeightMap = array('d')
        self.flowMap = array('i')
        self.averageRainfallMap = array('d')        
        self.drainageMap = array('d')
        self.riverMap = array('i')
        #initialize maps with zeros
        for i in range(0,hm.mapHeight*hm.mapWidth):
            self.averageHeightMap.append(0.0)
            self.flowMap.append(0)
            self.averageRainfallMap.append(0.0)
            self.drainageMap.append(0.0)
            self.riverMap.append(self.O)
        #Get highest intersection neighbor
        for y in range(hm.mapHeight):
            for x in range(hm.mapWidth):
                i = hm.GetIndex(x,y)
                maxHeight = 0.0;
                for yy in range(y,y-2,-1):
                    for xx in range(x,x+2):
                        ii = hm.GetIndex(xx,yy)
                        #use an average hight of <0 to denote an ocean border
                        #this will save processing time later
                        if(hm.plotMap[ii] == hm.OCEAN):
                            maxHeight = -100.0
                        elif maxHeight < hm.map[ii] and maxHeight >= 0:
                            maxHeight = hm.map[ii]
                self.averageHeightMap[i] = maxHeight
        #create flowMap by checking for the lowest of each 4 connected
        #neighbor plus self       
        for y in range(hm.mapHeight):
            for x in range(hm.mapWidth):
                i = hm.GetIndex(x,y)
                lowestAlt = self.averageHeightMap[i]
                if(lowestAlt < 0.0):
                    #if height is <0 then that means this intersection is
                    #adjacent to an ocean and has no flow
                    self.flowMap[i] = self.O
                else:
                    #First assume this place is lowest, like a 'pit'. Then
                    #for each place that is lower, add it to a list to be
                    #randomly chosen as the drainage path
                    drainList = list()
                    self.flowMap[i] = self.L 
                    ii = hm.GetIndex(x,y+1)
                    #in the y direction, avoid wrapping
                    if(y > 0 and self.averageHeightMap[ii] < lowestAlt):
                        drainList.append(self.N)
                    ii = hm.GetIndex(x,y-1)
                    if(y < hm.mapHeight - 1 and self.averageHeightMap[ii] < lowestAlt):
                        drainList.append(self.S)
                    ii = hm.GetIndex(x-1,y)
                    if(self.averageHeightMap[ii] < lowestAlt):
                        drainList.append(self.W)
                    ii = hm.GetIndex(x+1,y)
                    if(self.averageHeightMap[ii] < lowestAlt):
                        drainList.append(self.E)
                    count = len(drainList)
                    if count > 0:
                        choice = int(PWRand.random()*count)
#                        print count,choice
                        self.flowMap[i] = drainList[choice]
                  
        #Create average rainfall map so that each intersection is an average
        #of the rainfall from rm.rainMap
        for y in range(hm.mapHeight):
            for x in range(hm.mapWidth):
                i = hm.GetIndex(x,y)
                avg = 0.0;
                for yy in range(y,y-2,-1):
                    for xx in range(x,x+2):
                        ii = hm.GetIndex(xx,yy)
                        avg += rm.rainMap[ii]
                avg = avg/4.0
                self.averageRainfallMap[i] = avg
               
        #Now use the flowMap as a guide to distribute average rainfall.
        #Wherever the most rainfall ends up is where the rivers will be.
        print "Distributing rainfall"
        for y in range(hm.mapHeight):
            for x in range(hm.mapWidth):
                i = hm.GetIndex(x,y)
                flow = self.flowMap[i]
                rainFall = self.averageRainfallMap[i]
                xx = x
                yy = y
                while(flow != self.L and flow != self.O):
                    if(flow == self.N):
                        yy += 1
                    elif(flow == self.S):
                        yy -= 1
                    elif(flow == self.W):
                        xx -= 1
                    elif(flow == self.E):
                        xx += 1
                    #wrap
                    if(xx < 0):
                        xx = hm.mapWidth - 1
                    elif(xx >= hm.mapWidth):
                        xx = 0
                    if(yy < 0):
                        yy = hm.mapHeight - 1
                    elif(yy >= hm.mapHeight):
                        yy = 0
                    #dump rainfall here
                    ii = hm.GetIndex(xx,yy)
                    self.drainageMap[ii] += rainFall
                    #reset flow
                    flow = self.flowMap[ii]
                    
        #Normalize drainageMap to between 0.0 and 1.0            
#        NormalizeMap(hm.mapWidth,hm.mapHeight,self.drainageMap)
        
        riverThreshold = terr.plainsThreshold * mc.RiverThreshold
        for i in range(hm.mapHeight*hm.mapWidth):
            if(self.drainageMap[i] > riverThreshold):
##                    riverCount += 1
                self.riverMap[i] = self.flowMap[i]
            else:
                self.riverMap[i] = self.O

        #at this point river should be in tolerance or close to it
        #riverMap is ready for use
                
    def isFloodPlainThreshold(self,x,y):
        riverThreshold = terr.plainsThreshold * mc.RiverThreshold
        maxRain = 0
        #Loops are opposite of above examples because here we are going
        #from intersection to square instead of vice versa.
        for yy in range(y,y+2):
            for xx in range(x,x-2,-1):
                ii = hm.GetIndex(xx,yy)
                if maxRain < self.drainageMap[ii]:
                    maxRain = self.drainageMap[ii]
        if maxRain > riverThreshold * mc.FloodPlainThreshold:
            return True
        return False
##Global access
rivers = RiverMap()

class BonusPlacer :
    def __init__(self):
        return
    def AddBonuses(self):
        gc = CyGlobalContext()
        gameMap = CyMap()
        gameMap.recalculateAreas()
        self.AssignBonusAreas()
        numBonuses = gc.getNumBonusInfos()
        for order in range(numBonuses):
            for i in range(numBonuses):
                bonusInfo = gc.getBonusInfo(self.bonusList[i].eBonus)
                if bonusInfo.getPlacementOrder() == order:
                    self.AddBonusType(self.bonusList[i].eBonus)#Both unique and non-unique bonuses

        #now check to see that all resources have been placed at least once, this
        #pass ignoring area rules
        for i in range(numBonuses):
            bonus = self.bonusList[i]
            if bonus.currentBonusCount == 0 and bonus.desiredBonusCount > 0:
                self.AddEmergencyBonus(bonus,False)

        #now check again to see that all resources have been placed at least once,
        #this time ignoring area rules and also class spacing
        for i in range(numBonuses):
            bonus = self.bonusList[i]
            if bonus.currentBonusCount == 0 and bonus.desiredBonusCount > 0:
                self.AddEmergencyBonus(bonus,True)                                       

        #now report resources that simply could not be placed
        for i in range(numBonuses):
            bonus = self.bonusList[i]
            if bonus.currentBonusCount == 0 and bonus.desiredBonusCount > 0:
                bonusInfo = gc.getBonusInfo(bonus.eBonus)
                print "No room at all found for %(bt)s!!!" % {"bt":bonusInfo.getType()} 
        return
    def AddEmergencyBonus(self,bonus,ignoreClass):
        gc = CyGlobalContext()
        gameMap = CyMap()
        featureForest = gc.getInfoTypeForString("FEATURE_FOREST")
        plotIndexList = list()
        for i in range(hm.mapWidth*hm.mapHeight):
            plotIndexList.append(i)
        plotIndexList = ShuffleList(plotIndexList)
        bonusInfo = gc.getBonusInfo(bonus.eBonus)
        for i in range(len(plotIndexList)):
            index = plotIndexList[i]
            plot = gameMap.plotByIndex(index)
            x = plot.getX()
            y = plot.getY()
            if (ignoreClass and self.PlotCanHaveBonus(plot,bonus.eBonus,False,True)) or \
            self.CanPlaceBonusAt(plot,bonus.eBonus,False,True):
                #temporarily remove any feature
                featureEnum = plot.getFeatureType()
                if featureEnum == featureForest:
                    featureVariety = plot.getFeatureVariety()
                    plot.setFeatureType(FeatureTypes.NO_FEATURE,-1)
                #place bonus
                plot.setBonusType(bonus.eBonus)
                bonus.currentBonusCount += 1
                #restore the feature if possible
                if featureEnum == featureForest:
                    if bonusInfo == None or bonusInfo.isFeature(featureEnum):
                        plot.setFeatureType(featureEnum,featureVariety)
                print "Emergency placement of 1 %(bt)s" % {"bt":bonusInfo.getType()} 
                break
         
        return
    def AddBonusType(self,eBonus):
        gc = CyGlobalContext()
        gameMap = CyMap()
        featureForest = gc.getInfoTypeForString("FEATURE_FOREST")
        #first get bonus/area link
        for i in range(gc.getNumBonusInfos()):
            if self.bonusList[i].eBonus == eBonus:
                bonus = self.bonusList[i]
        bonus.currentBonusCount = 0
        bonusInfo = gc.getBonusInfo(eBonus)
        if bonus.desiredBonusCount <= 0:#Non map bonuses?
            print "Desired bonus count for %(bt)s is zero, so none will be placed." % {"bt":bonusInfo.getType()}
            return
        #Create a list of map indices and shuffle them
        preshuffle = list()
        plotIndexList = list()
        for i in range(hm.mapWidth*hm.mapHeight):
            preshuffle.append(i)
        for i in range(hm.mapWidth*hm.mapHeight):
            preshufflength = len(preshuffle)
            randIndex = PWRand.randint(0,len(preshuffle)-1)
            if randIndex < 0 or randIndex >= len(preshuffle):
                raise ValueError, " bad index shuffling plot list randIndex=%(r)d listLength=%(l)d, mapSize=%(s)d" % {"r":randIndex,"l":len(preshuffle),"s":hm.mapWidth*hm.mapHeight}
            if preshufflength != len(preshuffle):
                raise ValueError, "preshufflength != len(preshuffle) preshufflength=%(r)d listLength=%(l)d" % {"r":preshufflength,"l":len(preshuffle)}
            plotIndexList.append(preshuffle[randIndex])
            del preshuffle[randIndex]
            if len(preshuffle) == 0:
                break
        print "Trying to place %(n)d of %(bt)s" % {"n":bonus.desiredBonusCount,"bt":bonusInfo.getType()}
        #now add bonuses
        for i in range(len(plotIndexList)):
            index = plotIndexList[i]
            plot = gameMap.plotByIndex(index)
            x = plot.getX()
            y = plot.getY()
            if self.CanPlaceBonusAt(plot,eBonus,False,False):
                #temporarily remove any feature
                featureEnum = plot.getFeatureType()
                if featureEnum == featureForest:
                    featureVariety = plot.getFeatureVariety()
                    plot.setFeatureType(FeatureTypes.NO_FEATURE,-1)
                #place bonus
                plot.setBonusType(eBonus)
                bonus.currentBonusCount += 1
                #restore the feature if possible
                if featureEnum == featureForest:
                    if bonusInfo == None or bonusInfo.isFeature(featureEnum) or (mc.HiddenBonusInForest and bonusInfo.getTechReveal() != TechTypes.NO_TECH):
                        plot.setFeatureType(featureEnum,featureVariety)
                groupRange = bonusInfo.getGroupRange()
                for dx in range(-groupRange,groupRange + 1):
                    for dy in range(-groupRange,groupRange + 1):
                        if bonus.currentBonusCount < bonus.desiredBonusCount:
                            loopPlot = self.plotXY(x,y,dx,dy)
                            if loopPlot != None:
                                if loopPlot.getX() == -1:
                                    raise ValueError, "plotXY returns invalid plots plot= %(x)d, %(y)d" % {"x":x,"y":y}
                                if self.CanPlaceBonusAt(loopPlot,eBonus,False,False):
                                    if PWRand.randint(0,99) < bonusInfo.getGroupRand():
                                        #temporarily remove any feature
                                        featureEnum = loopPlot.getFeatureType()
                                        if featureEnum == featureForest:
                                            featureVariety = loopPlot.getFeatureVariety()
                                            loopPlot.setFeatureType(FeatureTypes.NO_FEATURE,-1)
                                        #place bonus
                                        loopPlot.setBonusType(eBonus)
                                        bonus.currentBonusCount += 1
                                        #restore the feature if possible
                                        if featureEnum == featureForest:
                                            if bonusInfo == None or bonusInfo.isFeature(featureEnum) or (mc.HiddenBonusInForest and bonusInfo.getTechReveal() != TechTypes.NO_TECH):
                                                loopPlot.setFeatureType(featureEnum,featureVariety)
            if bonus.currentBonusCount == bonus.desiredBonusCount:
                break
        if bonus.currentBonusCount < bonus.desiredBonusCount:
            print "Could not place %(n)d of %(bt)s" % {"n":bonus.desiredBonusCount - bonus.currentBonusCount,"bt":bonusInfo.getType()}
        else:
            print "Successfully placed %(n)d of %(bt)s" % {"n":bonus.desiredBonusCount,"bt":bonusInfo.getType()}
        return
    def plotXY(self,x,y,dx,dy):
        gameMap = CyMap()
        #The one that civ uses will return junk so I have to make one
        #that will not
        x = (x + dx) % hm.mapWidth
        y = y + dy
        if y < 0 or y > hm.mapHeight - 1:
            return None
        return gameMap.plot(x,y)
        
    def AssignBonusAreas(self):
        gc = CyGlobalContext()
        self.areas = CvMapGeneratorUtil.getAreas()
        gameMap = CyMap()
        preShuffledBonusList = list()
        self.bonusList = list()
        #Create and shuffle the bonus list and keep tally on
        #one-area bonuses and find the smallest min area requirement
        #among those
        numUniqueBonuses = 0
        minLandAreaSize = -1
        for i in range(gc.getNumBonusInfos()):
            bonus = BonusArea()
            bonus.eBonus = i
            preShuffledBonusList.append(bonus)
            bonusInfo = gc.getBonusInfo(i)
            if bonusInfo.isOneArea() == True:
                numUniqueBonuses += 1
                minAreaSize = bonusInfo.getMinAreaSize()
                if (minLandAreaSize == -1 or minLandAreaSize > minAreaSize) and \
                minAreaSize > 0:
                    minLandAreaSize = minAreaSize
        for i in range(gc.getNumBonusInfos()):
            iChooseBonus = PWRand.randint(0,len(preShuffledBonusList)-1)
            self.bonusList.append(preShuffledBonusList[iChooseBonus])
            del preShuffledBonusList[iChooseBonus]
        numBonuses = gc.getNumBonusInfos()
        for i in range(numBonuses):
            self.bonusList[i].desiredBonusCount = self.CalculateNumBonusesToAdd(self.bonusList[i].eBonus)
            bonusInfo = gc.getBonusInfo(self.bonusList[i].eBonus)
            eBonus = self.bonusList[i].eBonus
            if bonusInfo.isOneArea() == False:
                continue #Only assign areas to area bonuses
##            print "Trying to find room for %(bt)s, desiredCount=%(dc)d" % {"bt":bonusInfo.getType(),"dc":self.bonusList[i].desiredBonusCount}
            areaSuitabilityList = list()
            for area in self.areas:
                if area.getNumTiles() >= minLandAreaSize:
                    aS = AreaSuitability(area.getID())
                    aS.suitability,aS.numPossible = self.CalculateAreaSuitability(area,eBonus)
                    areaSuitabilityList.append(aS)
##                    print "suitability on areaID=%(aid)d, size=%(s)d is %(r)f" % \
##                    {"aid":area.getID(),"s":area.getNumTiles(),"r":aS.suitability}
            #Calculate how many areas to assign (numUniqueBonuses will be > 0 if we get here)
##            areasPerBonus = (len(areaSuitabilityList)/numUniqueBonuses) + 1
            areasPerBonus =  1
            #Sort areaSuitabilityList best first
            areaSuitabilityList.sort(lambda x,y:cmp(x.numPossible,y.numPossible))
            areaSuitabilityList.reverse()
            #assign the best areas to this bonus
            for n in range(areasPerBonus):
                self.bonusList[i].areaList.append(areaSuitabilityList[n].areaID)
            #assign areas that have a high suitability also
            for n in range(areasPerBonus,len(areaSuitabilityList)):
                if areaSuitabilityList[n].suitability > 0.3:
                    self.bonusList[i].areaList.append(areaSuitabilityList[n].areaID)
        return
    def CanPlaceBonusAt(self,plot,eBonus,bIgnoreLatitude,bIgnoreArea):
        gc = CyGlobalContext()
        gameMap = CyMap()
        x = plot.getX()
        y = plot.getY()
        areaID = plot.getArea()
        if self.PlotCanHaveBonus(plot,eBonus,bIgnoreLatitude,bIgnoreArea) == False:
            return False
        for i in range(DirectionTypes.NUM_DIRECTION_TYPES):
            loopPlot = plotDirection(x,y,DirectionTypes(i))
            if loopPlot.getBonusType(TeamTypes.NO_TEAM) != BonusTypes.NO_BONUS and loopPlot.getBonusType(TeamTypes.NO_TEAM) != eBonus:
               return False

        bonusInfo = gc.getBonusInfo(eBonus)
        classInfo = gc.getBonusClassInfo(bonusInfo.getBonusClassType())
        if plot.isWater() == True:
            if gameMap.getNumBonusesOnLand(eBonus) * 100/(gameMap.getNumBonuses(eBonus) + 1) < bonusInfo.getMinLandPercent():
                return False
        #Make sure there are no bonuses of the same class (but a different type) nearby:
        if classInfo != None:
            iRange = classInfo.getUniqueRange()
            for dx in range(-iRange,iRange+1):
                for dy in range(-iRange,iRange+1):
                    loopPlot = self.plotXY(x,y,dx,dy)
                    if loopPlot != None:
                        if areaID == loopPlot.getArea():
                            if plotDistance(x, y, loopPlot.getX(), loopPlot.getY()) <= iRange:
                                eOtherBonus = loopPlot.getBonusType(TeamTypes.NO_TEAM)
                                if eOtherBonus != BonusTypes.NO_BONUS:
                                    if gc.getBonusInfo(eOtherBonus).getBonusClassType() == bonusInfo.getBonusClassType():
                                        return False
        #Make sure there are no bonuses of the same type nearby:
        iRange = bonusInfo.getUniqueRange()
        for dx in range(-iRange,iRange+1):
            for dy in range(-iRange,iRange+1):
                loopPlot = self.plotXY(x,y,dx,dy)
                if loopPlot != None:
                    if areaID == loopPlot.getArea():
                        if plotDistance(x, y, loopPlot.getX(), loopPlot.getY()) <= iRange:
                            eOtherBonus = loopPlot.getBonusType(TeamTypes.NO_TEAM)
                            if eOtherBonus != BonusTypes.NO_BONUS:
                                if eOtherBonus == eBonus:
                                    return False
                           
                    
        return True
    def PlotCanHaveBonus(self,plot,eBonus,bIgnoreLatitude,bIgnoreArea):
        #This function is like CvPlot::canHaveBonus but will
        #ignore blocking features and checks for a valid area. 
        gc = CyGlobalContext()
        featureForest = gc.getInfoTypeForString("FEATURE_FOREST")
        if eBonus == BonusTypes.NO_BONUS:
            return True
        if plot.getBonusType(TeamTypes.NO_TEAM) != BonusTypes.NO_BONUS:
            return False
        if plot.isPeak() == True:
            return False
        bonusInfo = gc.getBonusInfo(eBonus)
        #Here is the change from canHaveBonus. Forest does not block bonus
        requiresForest = bonusInfo.isFeature(featureForest)
        plotIsForest = plot.getFeatureType() == featureForest
        #To avoid silk and spices on ice/tundra
        if plotIsForest and requiresForest:
            if bonusInfo.isFeatureTerrain(plot.getTerrainType()) == False:
                return False
        #now that bonuses that require forest are dealt with, count forest
        #as no feature
        else:
            if plot.getFeatureType() != FeatureTypes.NO_FEATURE and not plotIsForest:
                if bonusInfo.isFeature(plot.getFeatureType()) == False:
                    return False           
                if bonusInfo.isFeatureTerrain(plot.getTerrainType()) == False:
                    return False              
            else:
                if bonusInfo.isTerrain(plot.getTerrainType()) == False:
                    return False
                
        if plot.isHills() == True:
            if bonusInfo.isHills() == False:
                return False
        if plot.isFlatlands() == True:
            if bonusInfo.isFlatlands() == False:
                return False
        if bonusInfo.isNoRiverSide() == True:
            if plot.isRiverSide() == True:
                return False
        if bonusInfo.getMinAreaSize() != -1:
            if plot.area().getNumTiles() < bonusInfo.getMinAreaSize():
                return False
        if bIgnoreLatitude == False:
            if plot.getLatitude() > bonusInfo.getMaxLatitude():
                return False
            if plot.getLatitude() < bonusInfo.getMinLatitude():
                return False
        if plot.isPotentialCityWork() == False:
            return False
        
        if bIgnoreArea == False and bonusInfo.isOneArea() == True:
            areaID = plot.getArea()
            areaFound = False
            for i in range(len(self.bonusList)):
                if self.bonusList[i].eBonus == eBonus:
                    areaList = self.bonusList[i].areaList
                    for n in range(len(areaList)):
                        if areaList[n] == areaID:
                            areaFound = True
                            break
                    if areaFound == True:
                        break
            if areaFound == False:
                return False
                        
        return True
    def CalculateNumBonusesToAdd(self,eBonus):
        #This is like the function in CvMapGenerator except it uses
        #self.PlotCanHaveBonus instead of CvPlot::canHaveBonus
        gc = CyGlobalContext()
        gameMap = CyMap()
        game = CyGame()
        bonusInfo = gc.getBonusInfo(eBonus)
        rand1 = PWRand.randint(0,bonusInfo.getRandAppearance1())
        rand2 = PWRand.randint(0,bonusInfo.getRandAppearance2())
        rand3 = PWRand.randint(0,bonusInfo.getRandAppearance3())
        rand4 = PWRand.randint(0,bonusInfo.getRandAppearance4())
        baseCount = bonusInfo.getConstAppearance() + rand1 + rand2 + rand3 + rand4

        bIgnoreLatitude = False
        bIgnoreArea = True
        landTiles = 0
        numPossible = 0
        if bonusInfo.getTilesPer() > 0:
            for i in range(hm.mapWidth*hm.mapHeight):
                plot = gameMap.plotByIndex(i)
                if self.PlotCanHaveBonus(plot,eBonus,bIgnoreLatitude,bIgnoreArea):
                    numPossible += 1
            landTiles += numPossible/bonusInfo.getTilesPer()
        players = game.countCivPlayersAlive() * bonusInfo.getPercentPerPlayer()/100
        bonusCount = baseCount * (landTiles + players)/100
        bonusCount = max(1,bonusCount)
##        print "Calculating bonus amount for %(bt)s" % {"bt":bonusInfo.getType()}
##        print "baseCount=%(bc)d, numPossible=%(np)d, landTiles=%(lt)d, players=%(p)d" % \
##        {"bc":baseCount,"np":numPossible,"lt":landTiles,"p":players}
##        print ""
        return bonusCount
    
    def GetUniqueBonusTypeCountInArea(self,area):
        gc = CyGlobalContext()
        areaID = area.getID()
        uniqueBonusCount = 0
        for i in range(len(self.bonusList)):
            areaList = self.bonusList[i].areaList
            bonusInfo = gc.getBonusInfo(self.bonusList[i].eBonus)
            if bonusInfo.isOneArea() == False:
                continue
            for n in range(len(areaList)):
                if areaList[n] == areaID:
                    uniqueBonusCount += 1
                    break

        return uniqueBonusCount 
    
    def GetSameClassTypeCountInArea(self,area,eBonus):
        gc = CyGlobalContext()
        areaID = area.getID()
        uniqueBonusCount = 0
        bonusInfo = gc.getBonusInfo(eBonus)
        eClass = bonusInfo.getBonusClassType()
        if eClass == BonusClassTypes.NO_BONUSCLASS:
            return 0
        classInfo = gc.getBonusClassInfo(eClass)
        if classInfo == None:
            return 0
        uRange = classInfo.getUniqueRange()
        for i in range(len(self.bonusList)):
            areaList = self.bonusList[i].areaList
            bonusInfo = gc.getBonusInfo(self.bonusList[i].eBonus)
            if bonusInfo.isOneArea() == False:
                continue
            if bonusInfo.getBonusClassType() != eClass:
                continue
            for n in range(len(areaList)):
                if areaList[n] == areaID:
                    uniqueBonusCount += 1
                    break
        #Same class types tend to really crowd out any bonus
        #types that are placed later. A single cow can block
        #5 * 5 squares of pig territory for example. Probably
        #shouldn't place them on the same area at all, but
        #sometimes it might be necessary.
        return uniqueBonusCount * uRange * uRange
   
    def CalculateAreaSuitability(self,area,eBonus):
        gc = CyGlobalContext()
        gameMap = CyMap()
        areaID = area.getID()
        uniqueTypesInArea = self.GetUniqueBonusTypeCountInArea(area)
        sameClassTypesInArea = self.GetSameClassTypeCountInArea(area,eBonus)
        #Get the raw number of suitable tiles
        numPossible = 0
        for i in range(hm.mapWidth*hm.mapHeight):
            plot = gameMap.plotByIndex(i)
            if plot.getArea() == areaID:
                if self.PlotCanHaveBonus(plot,eBonus,False,True):
                    numPossible += 1
        numPossible = numPossible/(uniqueTypesInArea + sameClassTypesInArea + 1)
        suitability = float(numPossible)/float(area.getNumTiles())
        return suitability,numPossible
#Global Access
bp = BonusPlacer()

class BonusArea :
    def __init__(self):
        self.eBonus = -1
        self.desiredBonusCount = -1
        self.currentBonusCount = -1
        self.areaList = list()
            
class AreaSuitability :
    def __init__(self,areaID):
        self.areaID = areaID
        self.suitability = 0
        self.numPossible = 0

class StartingPlotFinder :
    def __init__(self):
        return
    def SetStartingPlots(self):
        gc = CyGlobalContext()
        gameMap = CyMap()
        iPlayers = gc.getGame().countCivPlayersEverAlive()
        gameMap.recalculateAreas()
        areas = CvMapGeneratorUtil.getAreas()

        #get old/new world status
        areaOldWorld = self.setupOldWorldAreaList()
        
        #Shuffle players so the same player doesn't always get the first pick.
        #lifted from Highlands.py that ships with Civ.
        player_list = []
        for plrCheckLoop in range(gc.getMAX_CIV_PLAYERS()):
                if CyGlobalContext().getPlayer(plrCheckLoop).isEverAlive():
                        player_list.append(plrCheckLoop)
        shuffledPlayers = []
        for playerLoop in range(iPlayers):
                iChoosePlayer = PWRand.randint(0,len(player_list)-1)
                shuffledPlayers.append(player_list[iChoosePlayer])
                del player_list[iChoosePlayer]

        self.startingAreaList = list()
        for i in range(len(areas)):
            if areaOldWorld[i] == True and areas[i].getNumTiles() > 5:
                startArea = StartingArea(areas[i].getID())
                self.startingAreaList.append(startArea)

        #Get the value of the whole old world
        oldWorldValue = 0
        for i in range(len(self.startingAreaList)):
            oldWorldValue += self.startingAreaList[i].rawValue

        #calulate value per player of old world
        oldWorldValuePerPlayer = oldWorldValue/len(shuffledPlayers)

        #Sort startingAreaList by rawValue
        self.startingAreaList.sort(lambda x, y: cmp(x.rawValue, y.rawValue))

        #Get rid of areas that have less value than oldWorldValuePerPlayer
        #as they are too small to put a player on, however leave at least
        #half as many continents as there are players, just in case the
        #continents are *all* quite small. 
        numAreas = max(1,len(self.startingAreaList) - len(shuffledPlayers)/2)
        for i in range(numAreas):
            if self.startingAreaList[0].rawValue < oldWorldValuePerPlayer:
                del self.startingAreaList[0]
            else:
                break #All remaining should be big enough
            
        #Recalculate the value of the whole old world
        oldWorldValue = 0
        for i in range(len(self.startingAreaList)):
            oldWorldValue += self.startingAreaList[i].rawValue

        #Recalulate value per player of old world so we are starting more
        #accurately
        oldWorldValuePerPlayer = oldWorldValue/len(shuffledPlayers)

        #Record the ideal number of players for each continent
        for startingArea in self.startingAreaList:
            startingArea.idealNumberOfPlayers = int(round(float(startingArea.rawValue)/float(oldWorldValuePerPlayer)))

        #Now we want best first
        self.startingAreaList.reverse()
        print "number of starting areas is %(s)3d" % {"s":len(self.startingAreaList)}

        iterations = 0
        while True:
            iterations += 1
            if iterations > 20:
                raise ValueError, "Too many iterations in starting area choosing loop."
            chosenStartingAreas = list()
            playersPlaced = 0
            #add up idealNumbers
            idealNumbers = 0
            for startingArea in self.startingAreaList:
                idealNumbers += startingArea.idealNumberOfPlayers
            if idealNumbers < len(shuffledPlayers):
                self.startingAreaList[0].idealNumberOfPlayers += 1
            elif idealNumbers > len(shuffledPlayers):
                self.startingAreaList[0].idealNumberOfPlayers -= 1

            #Choose areas
            for startingArea in self.startingAreaList:
                if startingArea.idealNumberOfPlayers + playersPlaced <= len(shuffledPlayers):
                    chosenStartingAreas.append(startingArea)
                    playersPlaced += startingArea.idealNumberOfPlayers
                    
            #add up idealNumbers again
            idealNumbers = 0
            for startingArea in chosenStartingAreas:
                idealNumbers += startingArea.idealNumberOfPlayers
            if idealNumbers == len(shuffledPlayers):
                break
            
        for startingArea in chosenStartingAreas:
            for i in range(startingArea.idealNumberOfPlayers):
                startingArea.playerList.append(shuffledPlayers[0])
                del shuffledPlayers[0]
            startingArea.FindStartingPlots()
                        
        if len(shuffledPlayers) > 0:
            raise ValueError,"Some players not placed in starting plot finder!"

        #Now set up for normalization
        self.plotList = list()
        for startingArea in self.startingAreaList:
            for i in range(len(startingArea.plotList)):
##                print len(startingArea.plotList)
                self.plotList.append(startingArea.plotList[i])
                
        #Remove bad features. (Jungle in the case of standard game)
        for i in range(len(self.plotList)):
            if self.plotList[i].vacant == True:
                continue
            plot = gameMap.plot(self.plotList[i].x,self.plotList[i].y)
            featureInfo = gc.getFeatureInfo(plot.getFeatureType())
            if featureInfo != None:
                totalYield = 0
                for y in range(YieldTypes.NUM_YIELD_TYPES):
                    totalYield += featureInfo.getYieldChange(YieldTypes(y))
                if totalYield <= 0:#bad feature
                    plot.setFeatureType(FeatureTypes.NO_FEATURE,-1)
            
            for n in range(DirectionTypes.NUM_DIRECTION_TYPES):
                loopPlot = plotDirection(self.plotList[i].x,self.plotList[i].y,DirectionTypes(n))
                if loopPlot == None:
                    continue
                featureInfo = gc.getFeatureInfo(loopPlot.getFeatureType())
                if featureInfo != None:
                    totalYield = 0
                    for y in range(YieldTypes.NUM_YIELD_TYPES):
                        totalYield += featureInfo.getYieldChange(YieldTypes(y))
                    if totalYield <= 0:#bad feature
                        loopPlot.setFeatureType(FeatureTypes.NO_FEATURE,-1)
                        
        #find the best totalValue
        self.plotList.sort(lambda x,y:cmp(x.totalValue,y.totalValue))
        self.plotList.reverse()
        bestTotalValue = self.plotList[0].totalValue
        print "bestTotalValue = %(b)6d" % {"b":bestTotalValue}
        for i in range(len(self.plotList)):
            if self.plotList[i].vacant == True:
                continue
            currentTotalValue = self.plotList[i].totalValue
            percentLacking = 1.0 - (float(currentTotalValue)/float(bestTotalValue))
            if percentLacking > 0:
                bonuses = min(5,int(percentLacking/0.2))
                print "boosting plot by %(bv)d" % \
                {"bv":bonuses}
                self.boostCityPlotValue(self.plotList[i].x,self.plotList[i].y,bonuses)        
        
        return
    
    def setupOldWorldAreaList(self):
        gc = CyGlobalContext()
        gameMap = CyMap()
        #get official areas and make corresponding lists that determines old
        #world vs. new and also the pre-settled value.
        areas = CvMapGeneratorUtil.getAreas()
        areaOldWorld = list()
        
        for i in range(len(areas)):
            for pI in range(hm.mapHeight*hm.mapWidth):
                plot = gameMap.plotByIndex(pI)
                if plot.getArea() == areas[i].getID():
                    if mc.AllowNewWorld and terr.cm.areaMap[pI] == terr.newWorldID:
                        areaOldWorld.append(False)#new world true = old world false
                    else:
                        areaOldWorld.append(True)
                    break

            
        return areaOldWorld
    def getCityPotentialValue(self,x,y):
        gc = CyGlobalContext()
        gameMap = CyMap()
#        game.gc.getGame()
        totalValue = 0
        totalFood = 0
        plot = gameMap.plot(x,y)
        if plot.isWater() == True:
            return 0,0
        if plot.isImpassable() == True:
            return 0,0
        cityPlotList = list()
        #The StartPlot class has a nifty function to determine
        #salt water vs. fresh
        sPlot = StartPlot(x,y,0)
        for i in range(gc.getNUM_CITY_PLOTS()):
            plot = plotCity(x,y,i)
            food,value = self.getPlotPotentialValue(plot.getX(),plot.getY(),sPlot.isCoast())
            totalFood += food
            cPlot = CityPlot(food,value)
            cityPlotList.append(cPlot)
        usablePlots = totalFood/gc.getFOOD_CONSUMPTION_PER_POPULATION()
        cityPlotList.sort(lambda x,y:cmp(x.value,y.value))
        cityPlotList.reverse()
        #value is obviously limited to available food
        if usablePlots > gc.getNUM_CITY_PLOTS():
            usablePlots = gc.getNUM_CITY_PLOTS()
        for i in range(usablePlots):
            cPlot = cityPlotList[i]
            totalValue += cPlot.value
        #The StartPlot class has a nifty function to determine
        #salt water vs. fresh
        sPlot = StartPlot(x,y,0)
        if sPlot.isCoast() == True:
            totalValue = int(float(totalValue) * mc.CoastalCityValueBonus)
                
        return totalFood,totalValue
    def getPlotPotentialValue(self,x,y,coastalCity):
        gc = CyGlobalContext()
        gameMap = CyMap()
        game = gc.getGame()
        debugOut = False
        value = 0
        commerce = 0
        food = 0
        production = 0
        plot = gameMap.plot(x,y)
        if debugOut: print "Evaluating plot x = %(x)d, y = %(y)d" % {"x":x,"y":y}
        commerce += plot.calculateBestNatureYield(YieldTypes.YIELD_COMMERCE,TeamTypes.NO_TEAM)
        food += plot.calculateBestNatureYield(YieldTypes.YIELD_FOOD,TeamTypes.NO_TEAM)
        production += plot.calculateBestNatureYield(YieldTypes.YIELD_PRODUCTION,TeamTypes.NO_TEAM)
        if debugOut: print "Natural yields. Food=%(f)d, Production=%(p)d, Commerce=%(c)d" % \
        {"f":food,"p":production,"c":commerce}
        #Get best bonus improvement score. Test tachnology era of bonus
        #first, then test each improvement
        bestImp = None
        bonusEnum = plot.getBonusType(TeamTypes.NO_TEAM) 
        bonusInfo = gc.getBonusInfo(bonusEnum)
        if bonusInfo != None and (gc.getTechInfo(bonusInfo.getTechCityTrade()) == None or \
        gc.getTechInfo(bonusInfo.getTechCityTrade()).getEra() <= game.getStartEra()) and \
        (gc.getTechInfo(bonusInfo.getTechReveal()) == None or \
        gc.getTechInfo(bonusInfo.getTechReveal()).getEra() <= game.getStartEra()): 
            if bonusInfo == None:
                if debugOut: print "Bonus Type = None"
            else:
                if debugOut: print "Bonus Type = %(b)s <------------------------------------------------------------" % {"b":bonusInfo.getType()}
            commerce += bonusInfo.getYieldChange(YieldTypes.YIELD_COMMERCE)
            food += bonusInfo.getYieldChange(YieldTypes.YIELD_FOOD)
            production += bonusInfo.getYieldChange(YieldTypes.YIELD_PRODUCTION)
            if debugOut: print "Bonus yields. Food=%(f)d, Production=%(p)d, Commerce=%(c)d" % \
            {"f":food,"p":production,"c":commerce}
        else:
            bonusEnum = -1
            bonusInfo = None
        improvementList = list()
        for n in range(gc.getNumBuildInfos()):
            #Test for improvement validity
            buildInfo = gc.getBuildInfo(n)
            impEnum = buildInfo.getImprovement()
            impInfo = gc.getImprovementInfo(impEnum)
            if impInfo == None:
                continue
            if gc.getTechInfo(buildInfo.getTechPrereq()).getEra() > game.getStartEra():
                if debugOut: print "Tech era not high enough for %(s)s" % {"s":impInfo.getType()}
                continue
            else:
                if debugOut: print "Tech is high enough for %(s)s" % {"s":impInfo.getType()}
            if plot.canHaveImprovement(impEnum,TeamTypes.NO_TEAM,True) == True:
                if debugOut: print "Plot can have %(s)s" % {"s":impInfo.getType()}
                #This function will not find bonus yield changes for NO_PLAYER much to my annoyance
                impCommerce = plot.calculateImprovementYieldChange(impEnum,YieldTypes.YIELD_COMMERCE,PlayerTypes.NO_PLAYER,False)
                impFood = plot.calculateImprovementYieldChange(impEnum,YieldTypes.YIELD_FOOD,PlayerTypes.NO_PLAYER,False)
                impProduction = plot.calculateImprovementYieldChange(impEnum,YieldTypes.YIELD_PRODUCTION,PlayerTypes.NO_PLAYER,False)

                if bonusEnum != -1:
                    impCommerce += impInfo.getImprovementBonusYield(bonusEnum,YieldTypes.YIELD_COMMERCE)
                    impFood += impInfo.getImprovementBonusYield(bonusEnum,YieldTypes.YIELD_FOOD)
                    impProduction += impInfo.getImprovementBonusYield(bonusEnum,YieldTypes.YIELD_PRODUCTION)
                #See if feature is removed, if so we must subtract the added yield
                #from that feature
                featureEnum = plot.getFeatureType()
                if featureEnum != -1 and buildInfo.isFeatureRemove(featureEnum) == True:
                    featureInfo = gc.getFeatureInfo(featureEnum)
                    if debugOut: print "Removing feature %(s)s" % {"s":featureInfo.getType()}
                    impCommerce -= (featureInfo.getYieldChange(YieldTypes.YIELD_COMMERCE) + \
                    featureInfo.getRiverYieldChange(YieldTypes.YIELD_COMMERCE) + \
                    featureInfo.getHillsYieldChange(YieldTypes.YIELD_COMMERCE))
                    
                    impFood -= (featureInfo.getYieldChange(YieldTypes.YIELD_FOOD) + \
                    featureInfo.getRiverYieldChange(YieldTypes.YIELD_FOOD) + \
                    featureInfo.getHillsYieldChange(YieldTypes.YIELD_FOOD))
                    
                    impProduction -= (featureInfo.getYieldChange(YieldTypes.YIELD_PRODUCTION) + \
                    featureInfo.getRiverYieldChange(YieldTypes.YIELD_PRODUCTION) + \
                    featureInfo.getHillsYieldChange(YieldTypes.YIELD_PRODUCTION))
                
                imp = Improvement(impEnum,impFood,impProduction,impCommerce,0)
                improvementList.append(imp)
                if debugOut: print "Improv yields. Food=%(f)d, Production=%(p)d, Commerce=%(c)d" % \
                {"f":impFood,"p":impProduction,"c":impCommerce}
            else:
                if debugOut: print "Plot can not have %(s)s" % {"s":impInfo.getType()}
                            
        for i in range(len(improvementList)):
            impCommerce = improvementList[i].commerce + commerce
            impFood = improvementList[i].food + food
            impProduction = improvementList[i].production + production
            impValue = impCommerce * mc.CommerceValue + impFood * mc.FoodValue + impProduction * mc.ProductionValue
            #Food surplus makes the square much more valueable than if there
            #is no food here.
            if food >= gc.getFOOD_CONSUMPTION_PER_POPULATION():
                impValue *= 4
            elif food == gc.getFOOD_CONSUMPTION_PER_POPULATION() - 1:
                impValue *= 2
            improvementList[i].value = impValue
        if len(improvementList) > 0:
            #sort all allowed improvement values to find the best
            improvementList.sort(lambda x,y:cmp(x.value,y.value))
            improvementList.reverse()
            bestImp = improvementList[0]
            if debugOut: print "bestImp.value=%(b)d" % {"b":bestImp.value}    
            commerce += bestImp.commerce
            food += bestImp.food
            production += bestImp.production
        else:
            if debugOut: print "no improvement possible here"
        value = commerce * mc.CommerceValue + food * mc.FoodValue + production * mc.ProductionValue
        if debugOut: print "Evaluating. Food=%(f)d, Production=%(p)d, Commerce=%(c)d" % \
        {"f":food,"p":production,"c":commerce}
        #Cheating to simulate a lighthouse
##        if coastalCity and plot.isWater():
##            food += 1
        #Food surplus makes the square much more valueable than if there
        #is no food here.
        if food >= gc.getFOOD_CONSUMPTION_PER_POPULATION():
            value *= 4
        elif food == gc.getFOOD_CONSUMPTION_PER_POPULATION() - 1:
            value *= 2
        if food + commerce + production < 3:
            value = 0
        if debugOut: print "Final Value = %(v)d" % {"v":value}
        if debugOut: print "****************************************************************"
            
        return food,value
    def boostCityPlotValue(self,x,y,bonuses):
        mapGen = CyMapGenerator()
        food,value = self.getCityPotentialValue(x,y)
        debugOut = False
        if debugOut: print "Value before boost = %(v)d" % {"v":value}
        gc = CyGlobalContext()
        gameMap = CyMap()
        game = gc.getGame()
        #Shuffle the bonus order so that different cities have different preferences
        #for bonuses
        bonusList = list()
        numBonuses = gc.getNumBonusInfos()
        for i in range(numBonuses):
            bonusList.append(i)
        shuffledBonuses = list()
        for i in range(numBonuses):
            n = PWRand.randint(0,len(bonusList) - 1)
            shuffledBonuses.append(bonusList[n])
            del bonusList[n]

        if len(shuffledBonuses) != numBonuses:
            raise ValueError, "Bad bonus shuffle. Learn 2 shuffle."

        bonusCount = 0
        
        #Do this process in 3 passes for each non-food yield type, randomly deciding
        #the order of the 3 types
        yields = []
        randomNum = PWRand.randint(0,2)
        if randomNum == 0:
            yields.append(YieldTypes.YIELD_PRODUCTION)
            yields.append(YieldTypes.YIELD_FOOD)
            yields.append(YieldTypes.YIELD_COMMERCE)
        elif randomNum == 1:
            yields.append(YieldTypes.YIELD_COMMERCE)
            yields.append(YieldTypes.YIELD_FOOD)
            yields.append(YieldTypes.YIELD_PRODUCTION)
        else:
            yields.append(YieldTypes.YIELD_FOOD)
            yields.append(YieldTypes.YIELD_COMMERCE)
            yields.append(YieldTypes.YIELD_PRODUCTION)
        for n in range(len(yields)):
            for i in range(gc.getNUM_CITY_PLOTS()):
                food,value = self.getCityPotentialValue(x,y)
                
                #switch to food if food is needed
                usablePlots = food/gc.getFOOD_CONSUMPTION_PER_POPULATION()
                if usablePlots <= bonusCount:
                    yields[n] = YieldTypes.YIELD_FOOD
                    
                if debugOut: print "value now at %(v)d" % {"v":value}
                if bonusCount >= bonuses:
                    if debugOut: print "Placed all bonuses."
                    if debugOut: print "****************************************************"
                    return
                plot = plotCity(x,y,i)
                if plot.getX() == x and plot.getY() == y:
                    continue
                if plot.getBonusType(TeamTypes.NO_TEAM) != BonusTypes.NO_BONUS:
                    continue
                #temporarily remove any feature
                featureEnum = plot.getFeatureType()
                if featureEnum != FeatureTypes.NO_FEATURE:
                    featureVariety = plot.getFeatureVariety()
                    plot.setFeatureType(FeatureTypes.NO_FEATURE,-1)
                for b in range(gc.getNumBonusInfos()):
                    bonusEnum = shuffledBonuses[b]
                    bonusInfo = gc.getBonusInfo(bonusEnum)
                    if bonusInfo.isNormalize() == False:
                        continue
                    if bonusInfo.getYieldChange(yields[n]) < 1:
                        continue
                    if bonusInfo.getTechCityTrade() == TechTypes.NO_TECH or \
                    gc.getTechInfo(bonusInfo.getTechCityTrade()).getEra() <= game.getStartEra():
                        if bp.PlotCanHaveBonus(plot,bonusEnum,False,False) == False:
                            if debugOut: print "Plot can't have %(b)s" % {"b":bonusInfo.getType()}
                            continue
                        if debugOut: print "Setting bonus type at %(x)d,%(y)d to %(b)s" % \
                        {"x":plot.getX(),"y":plot.getY(),"b":bonusInfo.getType()}
                        plot.setBonusType(bonusEnum)
                        bonusCount += 1
                        break
                #restore the feature if possible
                if featureEnum != FeatureTypes.NO_FEATURE:
                    bonusInfo = gc.getBonusInfo(plot.getBonusType(TeamTypes.NO_TEAM))
                    if bonusInfo == None or bonusInfo.isFeature(featureEnum):
                        plot.setFeatureType(featureEnum,featureVariety)
                        
                                                
        if debugOut: print "Failed to boost city value. value= %(v)d" % {"v":value}
        if debugOut: print "****************************************************"
        return
#Global access
spf = StartingPlotFinder()
class CityPlot :
    def __init__(self,food,value):
        self.food = food
        self.value = value
class Improvement :
    def __init__(self,e,food,production,commerce,value):
        self.e = e
        self.food = food
        self.production = production
        self.commerce = commerce
        self.value = value

class StartingArea :
    def __init__(self,areaID):
        self.areaID = areaID
        self.playerList = list()
        self.plotList = list()
        self.distanceTable = array('i')
        self.rawValue = 0
        self.CalculatePlotList()
        self.idealNumberOfPlayers = 0
        return
    def CalculatePlotList(self):
        gc = CyGlobalContext()
        gameMap = CyMap()
        
        for y in range(hm.mapHeight):
            for x in range(hm.mapWidth):
                plot = gameMap.plot(x,y)
                if plot.getArea() == self.areaID:
                    #don't place a city on top of a bonus
                    if plot.getBonusType(TeamTypes.NO_TEAM) != BonusTypes.NO_BONUS:
                        continue
                    food,value = spf.getCityPotentialValue(x,y)
                    if value > 0:
                        startPlot = StartPlot(x,y,value)
                        if plot.isWater() == True:
                            raise ValueError, "potential start plot is water!"
                        self.plotList.append(startPlot)
        #Sort plots by local value
        self.plotList.sort(lambda x, y: cmp(x.localValue, y.localValue))
        
        #To save time and space let's get rid of some of the lesser plots
        cull = (len(self.plotList) * 2) / 3
        for i in range(cull):
            del self.plotList[0]
            
        #You now should be able to eliminate more plots by sorting high to low
        #and having the best plot eat plots within 3 squares, then same for next,
        #etc.
        self.plotList.reverse()
##        print "number of initial plots in areaID = %(a)3d is %(p)5d" % {"a":self.areaID,"p":len(self.plotList)}
        numPlots = len(self.plotList)
        for n in range(numPlots):
            #At some point the length of plot list will be much shorter than at
            #the beginning of the loop, so it can never end normally
            if n >= len(self.plotList) - 1:
                break
            y = self.plotList[n].y
            x = self.plotList[n].x
            for yy in range(y - 3,y + 4):
                for xx in range(x - 3,x + 4):
                    if yy < 0 or yy >= hm.mapHeight:
                        continue
                    xx = xx % hm.mapWidth#wrap xx
                    if xx < 0:
                        raise ValueError, "xx value not wrapping properly in StartingArea.CalculatePlotList"
                    for m in range(n,len(self.plotList)):
                        #At some point the length of plot list will be much shorter than at
                        #the beginning of the loop, so it can never end normally
                        if m >= len(self.plotList) - 1:
                            break
##                        print "m = %(m)3d, n = %(n)3d" % {"m":m,"n":n}
                        if self.plotList[m] != self.plotList[n]:
                            if self.plotList[m].x == xx and self.plotList[m].y == yy:
##                                print "deleting m = %(m)3d" % {"m":m}
                                del self.plotList[m]
##                                print "length of plotList now %(len)4d" % {"len":len(self.plotList)}
                                 
##        print "number of final plots in areaID = %(a)3d is %(p)5d" % {"a":self.areaID,"p":len(self.plotList)}
                                
        #At this point we should have a list of the very best places
        #to build cities on this continent. Now we need a table with
        #the distance from each city to every other city

        #Create distance table
        for i in range(len(self.plotList)*len(self.plotList)):
            self.distanceTable.append(-11)
        #Fill distance table
        for n in range(len(self.plotList)):
            #While were already looping lets calculate the raw value
            self.rawValue += self.plotList[n].localValue
            avgDistance = 0
            for m in range(n,len(self.plotList)):
                nPlot = gameMap.plot(self.plotList[n].x,self.plotList[n].y)
                mPlot = gameMap.plot(self.plotList[m].x,self.plotList[m].y)
                gameMap.resetPathDistance()
                distance = gameMap.calculatePathDistance(nPlot,mPlot)
#               distance = self.getDistance(nPlot.getX(),nPlot.getY(),mPlot.getX(),mPlot.getY())
                #If path fails try reversing it
##                gameMap.resetPathDistance()
##                newDistance = gameMap.calculatePathDistance(mPlot,nPlot)
##                if distance != newDistance:
##                    print "distance between n=%(n)d nx=%(nx)d,ny=%(ny)d and m=%(m)d mx=%(mx)d,my=%(my)d is %(d)d or %(nd)d" % \
##                    {"n":n,"nx":nPlot.getX(),"ny":nPlot.getY(),"m":m,"mx":mPlot.getX(),"my":mPlot.getY(),"d":distance,"nd":newDistance}
                self.distanceTable[n*len(self.plotList) + m] = distance
                self.distanceTable[m*len(self.plotList) + n] = distance
                avgDistance += distance
            self.plotList[n].avgDistance = avgDistance
 
        return
    def FindStartingPlots(self):
        gc = CyGlobalContext()
        gameMap = CyMap()
        numPlayers = len(self.playerList)
        if numPlayers <= 0:
            return

        avgDistanceList = list()
        for i in range(len(self.plotList)):
            avgDistanceList.append(self.plotList[i])
            
        #Make sure first guy starts on the end and not in the middle,
        #otherwise if there are two players one will start on the middle
        #and the other on the end
        avgDistanceList.sort(lambda x,y:cmp(x.avgDistance,y.avgDistance))
        avgDistanceList.reverse()
        #First place players as far as possible away from each other
        #Place the first player
        avgDistanceList[0].vacant = False
        for i in range(1,numPlayers):
            distanceList = list()
            for n in range(len(self.plotList)):
                if self.plotList[n].vacant == True:
                    minDistance = -1
                    for m in range(len(self.plotList)):
                        if self.plotList[m].vacant == False:
                            ii = n * len(self.plotList) + m
                            distance = self.distanceTable[ii]
                            if minDistance == -1 or minDistance > distance:
                                minDistance = distance
                    self.plotList[n].nearestStart = minDistance
                    distanceList.append(self.plotList[n])
            #Find biggest nearestStart and place a start there
            distanceList.sort(lambda x,y:cmp(x.nearestStart,y.nearestStart))
            distanceList.reverse()
            distanceList[0].vacant = False
##            print "Placing start at x=%(x)d, y=%(y)d nearestDistance to city is %(n)d" % \
##            {"x":distanceList[0].x,"y":distanceList[0].y,"n":distanceList[0].nearestStart}
                
        self.CalculateStartingPlotValues()
                
##        self.PrintPlotMap()
##        self.PrintPlotList()
##        self.PrintDistanceTable()
      
        #Now place all starting positions
        n = 0
        for m in range(len(self.plotList)):
            if self.plotList[m].vacant == False:
                sPlot = gameMap.plot(self.plotList[m].x,self.plotList[m].y)
                if sPlot.isWater() == True:
                    raise ValueError, "Start plot is water!"
                sPlot.setImprovementType(gc.getInfoTypeForString("NO_IMPROVEMENT"))
                playerID = self.playerList[n]
                player = gc.getPlayer(playerID)
                sPlot.setStartingPlot(True)
                player.setStartingPlot(sPlot,True)
                n += 1

            
        return
    def CalculateStartingPlotValues(self):
        gameMap = CyMap()
        numPlots = len(self.plotList)
        
        for n in range(numPlots):
            self.plotList[n].owner = -1
            self.plotList[n].totalValue = 0
            
        for n in range(numPlots):
            if self.plotList[n].vacant == True:
                continue
            self.plotList[n].totalValue = 0
            self.plotList[n].numberOfOwnedCities = 0
            for m in range(numPlots):
                i = n * numPlots + m
                nDistance = self.distanceTable[i]
                if nDistance == -1:
                    leastDistance = False
                else:
                    leastDistance = True #Being optimistic, prove me wrong
                for p in range(numPlots):
                    if p == n or self.plotList[p].vacant == True:
                        continue
                    ii = p * numPlots + m
                    pDistance = self.distanceTable[ii]
##                    print "n= %(n)3d, m = %(m)3d, p = %(p)3d, nDistance = %(nd)3d, pDistance = %(pd)3d" %\
##                    {"n":n,"m":m,"p":p,"nd":nDistance,"pd":pDistance}
                    if pDistance > -1 and pDistance <= nDistance:
                        leastDistance = False #Proven wrong
                        break
                    
                if leastDistance == True:
                    self.plotList[n].totalValue += self.plotList[m].localValue
#                    print "m = %(m)3d owner change from %(mo)3d to %(n)3d" % {"m":m,"mo":self.plotList[m].owner,"n":n}
                    self.plotList[m].owner = self.plotList[n]
                    self.plotList[m].distanceToOwner = nDistance
                    self.plotList[n].numberOfOwnedCities += 1
                    
        return
    def getDistance(self,x,y,dx,dy):
        xx = x - dx
        if abs(xx) > hm.mapWidth/2:
            xx = hm.mapWidth - abs(xx)
            
        distance = max(abs(xx),abs(y - dy))
        return distance
    def PrintPlotMap(self):
        gameMap = CyMap()
        print "Starting Plot Map"
        for y in range(hm.mapHeight - 1,-1,-1):
            lineString = ""
            for x in range(hm.mapWidth):
                inList = False
                for n in range(len(self.plotList)):
                    if self.plotList[n].x == x and self.plotList[n].y == y:
                        if self.plotList[n].plot().isWater() == True:
                            if self.plotList[n].vacant == True:
                                lineString += 'VV'
                            else:
                                lineString += 'OO'
                        else:
                            if self.plotList[n].vacant == True:
                                lineString += 'vv'
                            else:
                                lineString += 'oo'
                        inList = True
                        break
                if inList == False:
                    plot = gameMap.plot(x,y)
                    if plot.isWater() == True:
                        lineString += '.;'
                    else:
                        lineString += '[]'

            lineString += "-" + str(y)
            print lineString
            
        lineString = " "
        print lineString

        return
    def PrintPlotList(self):
        for n in range(len(self.plotList)):
            print str(n) + ' ' + str(self.plotList[n])
        return
    
    def PrintDistanceTable(self):
        print "Distance Table"
        lineString = "%(n)05d" % {"n":0} + ' '
        for n in range(len(self.plotList)):
            lineString += "%(n)05d" % {"n":n} + ' '
        print lineString
        lineString = ""
        for n in range(len(self.plotList)):
            lineString = "%(n)05d" % {"n":n} + ' '
            for m in range(len(self.plotList)):
                i = n * len(self.plotList) + m
                lineString += "%(d)05d" % {"d":self.distanceTable[i]} + ' '
            print lineString    
        return

class StartPlot :
    def __init__(self,x,y,localValue):
        self.x = x
        self.y = y
        self.localValue = localValue
        self.totalValue = 0
        self.numberOfOwnedCities = 0
        self.distanceToOwner = -1
        self.nearestStart = -1
        self.vacant = True
        self.owner = None
        self.avgDistance = 0
        return
    def isCoast(self):
        gameMap = CyMap()
        plot = gameMap.plot(self.x,self.y)
        waterArea = plot.waterArea()
        if waterArea.isNone() == True or waterArea.isLake() == True: 
            return False
        return True
    def plot(self):
        gameMap = CyMap()
        return gameMap.plot(self.x,self.y)
    def copy(self):
        cp = StartPlot(self.x,self.y,self.localValue)
        cp.totalValue = self.totalValue
        cp.numberOfOwnedCities = self.numberOfOwnedCities
        cp.distanceToOwner = self.distanceToOwner
        cp.nearestStart = self.nearestStart
        cp.vacant = self.vacant
        cp.owner = self.owner
        cp.avgDistance = self.avgDistance
        return cp
    def __str__(self):
        linestring = "x=%(x)3d,y=%(y)3d,localValue=%(lv)6d,totalValue =%(tv)6d, nearestStart=%(ad)6d, coastalCity=%(cc)d" % \
        {"x":self.x,"y":self.y,"lv":self.localValue,"tv":self.totalValue,"ad":self.nearestStart,"cc":self.isCoast()}
        return linestring
'''
This function examines a lake area and removes ugly surrounding rivers. Any
river that is flowing away from the lake, or alongside the lake will be
removed. This function also returns a list of riverID's that flow into the
lake.
'''
def cleanUpLake(x,y):
    gc = CyGlobalContext()
    mmap = gc.getMap()
    riversIntoLake = list()
    plot = mmap.plot(x,y+1)#North
    if plot != 0 and plot.isNOfRiver() == True:
        plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    if plot != 0 and plot.isWOfRiver() == True:
        if plot.getRiverNSDirection() == CardinalDirectionTypes.CARDINALDIRECTION_SOUTH:
            riversIntoLake.append(plot.getRiverID())
        else:
            plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    plot = mmap.plot(x - 1,y)#West
    if plot != 0 and plot.isWOfRiver() == True:
        plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    if plot != 0 and plot.isNOfRiver() == True:
        if plot.getRiverWEDirection() == CardinalDirectionTypes.CARDINALDIRECTION_EAST:
            riversIntoLake.append(plot.getRiverID())
        else:
            plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    plot = mmap.plot(x + 1,y)#East
    if plot != 0 and plot.isNOfRiver() == True:
        if plot.getRiverWEDirection() == CardinalDirectionTypes.CARDINALDIRECTION_WEST:
            riversIntoLake.append(plot.getRiverID())
        else:
            plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    plot = mmap.plot(x,y-1)#South
    if plot != 0 and plot.isWOfRiver() == True:
        if plot.getRiverNSDirection() == CardinalDirectionTypes.CARDINALDIRECTION_NORTH:
            riversIntoLake.append(plot.getRiverID())
        else:
            plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    plot = mmap.plot(x-1,y+1)#Northwest
    if plot != 0 and plot.isWOfRiver() == True:
        if plot.getRiverNSDirection() == CardinalDirectionTypes.CARDINALDIRECTION_SOUTH:
            riversIntoLake.append(plot.getRiverID())
        else:
            plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    if plot != 0 and plot.isNOfRiver() == True:
        if plot.getRiverWEDirection() == CardinalDirectionTypes.CARDINALDIRECTION_EAST:
            riversIntoLake.append(plot.getRiverID())
        else:
            plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    plot = mmap.plot(x+1,y+1)#Northeast
    if plot != 0 and plot.isNOfRiver() == True:
        if plot.getRiverWEDirection() == CardinalDirectionTypes.CARDINALDIRECTION_WEST:
            riversIntoLake.append(plot.getRiverID())
        else:
            plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    plot = mmap.plot(x-1,y-1)#Southhwest
    if plot != 0 and plot.isWOfRiver() == True:
        if plot.getRiverNSDirection() == CardinalDirectionTypes.CARDINALDIRECTION_NORTH:
            riversIntoLake.append(plot.getRiverID())
        else:
            plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    #Southeast plot is not relevant 
            
    return riversIntoLake
'''
This function replaces rivers to update the river crossings after a lake or
channel is placed at X,Y. There had been a long standing problem where water tiles
added after a river were causing graphical glitches and incorrect river rules
due to not updating the river crossings.
'''
def replaceRivers(x,y):
    gc = CyGlobalContext()
    mmap = gc.getMap()
    plot = mmap.plot(x,y+1)#North
    if plot != 0 and plot.isWOfRiver() == True:
        if plot.getRiverNSDirection() == CardinalDirectionTypes.CARDINALDIRECTION_SOUTH:
            #setting the river to what it already is will be ignored by the dll,
            #so it must be unset and then set again.
            plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
            plot.setWOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_SOUTH)
    plot = mmap.plot(x - 1,y)#West
    if plot != 0 and plot.isNOfRiver() == True:
        if plot.getRiverWEDirection() == CardinalDirectionTypes.CARDINALDIRECTION_EAST:
            plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
            plot.setNOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_EAST)
    plot = mmap.plot(x + 1,y)#East
    if plot != 0 and plot.isNOfRiver() == True:
        if plot.getRiverWEDirection() == CardinalDirectionTypes.CARDINALDIRECTION_WEST:
            plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
            plot.setNOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_WEST)
    plot = mmap.plot(x,y-1)#South
    if plot != 0 and plot.isWOfRiver() == True:
        if plot.getRiverNSDirection() == CardinalDirectionTypes.CARDINALDIRECTION_NORTH:
            plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
            plot.setWOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_NORTH)
    plot = mmap.plot(x-1,y+1)#Northwest
    if plot != 0 and plot.isWOfRiver() == True:
        if plot.getRiverNSDirection() == CardinalDirectionTypes.CARDINALDIRECTION_SOUTH:
            plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
            plot.setWOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_SOUTH)
    if plot != 0 and plot.isNOfRiver() == True:
        if plot.getRiverWEDirection() == CardinalDirectionTypes.CARDINALDIRECTION_EAST:
            plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
            plot.setNOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_EAST)
    plot = mmap.plot(x+1,y+1)#Northeast
    if plot != 0 and plot.isNOfRiver() == True:
        if plot.getRiverWEDirection() == CardinalDirectionTypes.CARDINALDIRECTION_WEST:
            plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
            plot.setNOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_WEST)
    plot = mmap.plot(x-1,y-1)#Southhwest
    if plot != 0 and plot.isWOfRiver() == True:
        if plot.getRiverNSDirection() == CardinalDirectionTypes.CARDINALDIRECTION_NORTH:
            plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
            plot.setWOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_NORTH)
    #Southeast plot is not relevant 
            
    return

'''
It looks bad to have a lake, fed by a river, sitting right next to the coast.
This function tries to minimize that occurance by replacing it with a
natural harbor, which looks much better.
'''
def makeHarbor(x,y,oceanMap):
    oceanID = oceanMap.getOceanID()
    i = oceanMap.getIndex(x,y)
    if oceanMap.areaMap[i] != oceanID:
        return
    #N
    xx = x
    yy = y + 2
    ii = oceanMap.getIndex(xx,yy)
    if ii > -1 and \
    oceanMap.getAreaByID(oceanMap.areaMap[ii]).water == True and \
    oceanMap.areaMap[ii] != oceanID:
        makeChannel(x,y + 1)
        oceanMap.defineAreas(False)
        oceanID = oceanMap.getOceanID()
    #S
    xx = x
    yy = y - 2
    ii = oceanMap.getIndex(xx,yy)
    if ii > -1 and \
    oceanMap.getAreaByID(oceanMap.areaMap[ii]).water == True and \
    oceanMap.areaMap[ii] != oceanID:
        makeChannel(x,y - 1)
        oceanMap.defineAreas(False)
        oceanID = oceanMap.getOceanID()
    #E
    xx = x + 2
    yy = y 
    ii = oceanMap.getIndex(xx,yy)
    if ii > -1 and \
    oceanMap.getAreaByID(oceanMap.areaMap[ii]).water == True and \
    oceanMap.areaMap[ii] != oceanID:
        makeChannel(x + 1,y)
        oceanMap.defineAreas(False)
        oceanID = oceanMap.getOceanID()
    #W
    xx = x - 2
    yy = y 
    ii = oceanMap.getIndex(xx,yy)
    if ii > -1 and \
    oceanMap.getAreaByID(oceanMap.areaMap[ii]).water == True and \
    oceanMap.areaMap[ii] != oceanID:
        makeChannel(x - 1,y)
        oceanMap.defineAreas(False)
        oceanID = oceanMap.getOceanID()
    #NW
    xx = x - 1
    yy = y + 1
    ii = oceanMap.getIndex(xx,yy)
    if ii > -1 and \
    oceanMap.getAreaByID(oceanMap.areaMap[ii]).water == True and \
    oceanMap.areaMap[ii] != oceanID:
        makeChannel(x - 1,y)
        oceanMap.defineAreas(False)
        oceanID = oceanMap.getOceanID()
    #NE
    xx = x + 1
    yy = y + 1
    ii = oceanMap.getIndex(xx,yy)
    if ii > -1 and \
    oceanMap.getAreaByID(oceanMap.areaMap[ii]).water == True and \
    oceanMap.areaMap[ii] != oceanID:
        makeChannel(x + 1,y)
        oceanMap.defineAreas(False)
        oceanID = oceanMap.getOceanID()
    #SW
    xx = x - 1
    yy = y - 1
    ii = oceanMap.getIndex(xx,yy)
    if ii > -1 and \
    oceanMap.getAreaByID(oceanMap.areaMap[ii]).water == True and \
    oceanMap.areaMap[ii] != oceanID:
        makeChannel(x ,y - 1)
        oceanMap.defineAreas(False)
        oceanID = oceanMap.getOceanID()
    #NW
    xx = x - 1
    yy = y + 1
    ii = oceanMap.getIndex(xx,yy)
    if ii > -1 and \
    oceanMap.getAreaByID(oceanMap.areaMap[ii]).water == True and \
    oceanMap.areaMap[ii] != oceanID:
        makeChannel(x,y + 1)
        oceanMap.defineAreas(False)
        oceanID = oceanMap.getOceanID()
    return
def makeChannel(x,y):
    gc = CyGlobalContext()
    mmap = gc.getMap()
    terrainCoast = gc.getInfoTypeForString("TERRAIN_COAST")
    plot = mmap.plot(x,y)
    cleanUpLake(x,y)
    plot.setTerrainType(terrainCoast,True,True)
    plot.setRiverID(-1)
    plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    replaceRivers(x,y)
    i = hm.GetIndex(x,y)
    hm.plotMap[i] = hm.OCEAN
    return
def expandLake(x,y,riversIntoLake,oceanMap):
    class LakePlot :
        def __init__(self,x,y,altitude):
            self.x = x
            self.y = y
            self.altitude = altitude
    gc = CyGlobalContext()
    mmap = gc.getMap()
    terrainCoast = gc.getInfoTypeForString("TERRAIN_COAST")
    lakePlots = list()
    lakeNeighbors = list()
    lakeSize = 2
    for i in riversIntoLake:
        lakeSize += riverList[i]
    i = oceanMap.getIndex(x,y)
    desertModifier = 1.0
    if terr.terrainMap[i] == terr.DESERT:
        desertModifier = mc.DesertLakeModifier
    lakeSize = int(lakeSize * mc.LakeSizePerRiverLength * desertModifier + 0.9999)
    start = LakePlot(x,y,hm.map[i])
    lakeNeighbors.append(start)
#    print "lakeSize",lakeSize
    while lakeSize > 0:
#        lakeNeighbors.sort(key=operator.attrgetter('altitude'),reverse=False)
        lakeNeighbors.sort(lambda x,y:cmp(x.altitude,y.altitude))
        currentLakePlot = lakeNeighbors[0]
        del lakeNeighbors[0]
        lakePlots.append(currentLakePlot)
        plot = mmap.plot(currentLakePlot.x,currentLakePlot.y)
        #if you are erasing a river to make a lake, make the lake smaller
        if plot.isNOfRiver() == True or plot.isWOfRiver() == True:
            lakeSize -= 1
        makeChannel(currentLakePlot.x,currentLakePlot.y)
        #Add valid neighbors to lakeNeighbors
        for n in range(4):
            if n == 0:#N
                xx = currentLakePlot.x
                yy = currentLakePlot.y + 1
                ii = oceanMap.getIndex(xx,yy)
            elif n == 1:#S
                xx = currentLakePlot.x
                yy = currentLakePlot.y - 1
                ii = oceanMap.getIndex(xx,yy)
            elif n == 2:#E
                xx = currentLakePlot.x + 1
                yy = currentLakePlot.y
                ii = oceanMap.getIndex(xx,yy)
            elif n == 3:#W
                xx = currentLakePlot.x - 1
                yy = currentLakePlot.y 
                ii = oceanMap.getIndex(xx,yy)
            else:
                raise ValueError, "too many cardinal directions"
            if ii != -1:
                #if this neighbor is in water area, then quit
                areaID = oceanMap.areaMap[ii]
                if areaID == 0:
                    raise ValueError, "areaID = 0 while generating lakes. This is a bug"
                for n in range(len(oceanMap.areaList)):
                    if oceanMap.areaList[n].ID == areaID:
                        if oceanMap.areaList[n].water == True:
#                            print "lake touched waterID = %(id)3d with %(ls)3d squares unused" % {'id':areaID,'ls':lakeSize}
#                            print "n = %(n)3d" % {"n":n}
#                            print str(oceanMap.areaList[n])
                            return
                if rivers.riverMap[ii] != rivers.L and mmap.plot(xx,yy).isWater() == False:
                    lakeNeighbors.append(LakePlot(xx,yy,hm.map[ii]))
        
        lakeSize -= 1
#    print "lake finished normally at %(x)2d,%(y)2d" % {"x":x,"y":y}
    return
def ShuffleList(theList):
        preshuffle = list()
        shuffled = list()
        numElements = len(theList)
        for i in range(numElements):
            preshuffle.append(theList[i])
        for i in range(numElements):
                n = PWRand.randint(0,len(preshuffle)-1)
                shuffled.append(preshuffle[n])
                del preshuffle[n]
        return shuffled
###############################################################################     
#functions that civ is looking for
###############################################################################
def getDescription():
	"""
	A map's Description is displayed in the main menu when players go to begin a game.
	For no description return an empty string.
	"""
	return "Random map that simulates earth-like plate tectonics, " +\
        "geostrophic winds and rainfall. Also, this map has a high likelyhood " +\
        "of 'New World' type lands only reachable on the open ocean. Be warned " +\
        "that these new lands are often home to terrifying hordes of " +\
        "mace-wielding barbarians. Any explorers you send here should " +\
        "be thoroughly briefed on the risks involved prior to embarcation. " +\
        "Use of the 'buddy system' is encouraged."
    
#This doesn't work with my river system so it is disabled. Some civs
#might start without a river. Boo hoo.
def normalizeAddRiver():
    return
def normalizeAddLakes():
    return
def normalizeAddGoodTerrain():
    return
def normalizeRemoveBadTerrain():
    return
def normalizeRemoveBadFeatures():
    return
def normalizeAddFoodBonuses():
    return
def normalizeAddExtras():
    return
def addLakes():
    print "Adding Lakes"
    gc = CyGlobalContext()
    mmap = gc.getMap()
    terrainCoast = gc.getInfoTypeForString("TERRAIN_COAST")
#    PrintFlowMap()
    oceanMap = Areamap(hm.mapWidth,hm.mapHeight)
    oceanMap.defineAreas(False)
##    oceanMap.PrintList(oceanMap.areaList)
##    oceanMap.PrintAreaMap()
    for y in range(hm.mapHeight):
        for x in range(hm.mapWidth):
            i = hm.GetIndex(x,y)
            if rivers.flowMap[i] == rivers.L:
                riversIntoLake = cleanUpLake(x,y)
                plot = mmap.plot(x,y)
                if len(riversIntoLake) > 0:
##                    plot.setRiverID(-1)
##                    plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
##                    plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
##                    #plot.setPlotType(PlotTypes.PLOT_OCEAN,True,True) setTerrain handles this already
##                    plot.setTerrainType(terrainCoast,True,True)
                    expandLake(x,y,riversIntoLake,oceanMap)
                else:
                    #no lake here, but in that case there should be no rivers either
                    plot.setRiverID(-1)
                    plot.setNOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
                    plot.setWOfRiver(False,CardinalDirectionTypes.NO_CARDINALDIRECTION)
    oceanMap.defineAreas(False)
##    oceanMap.PrintList(oceanMap.areaList)
##    oceanMap.PrintAreaMap()
    for y in range(hm.mapHeight):
        for x in range(hm.mapWidth):
            i = hm.GetIndex(x,y)
            makeHarbor(x,y,oceanMap)
    return
def isAdvancedMap():
	"""
	Advanced maps only show up in the map script pulldown on the advanced menu.
	Return 0 if you want your map to show up in the simple singleplayer menu
	"""
	return 0
def isClimateMap():
	"""
	Uses the Climate options
	"""
	return 0
	
def isSeaLevelMap():
	"""
	Uses the Sea Level options
	"""
	return 0
def getNumCustomMapOptions():
	"""
	Number of different user-defined options for this map
	Return an integer
	"""
	return 2
	
def getCustomMapOptionName(argsList):
        """
        Returns name of specified option
        argsList[0] is Option ID (int)
        Return a Unicode string
        """
        optionID = argsList[0]
        if optionID == 0:
            return "New World Rules"
        elif optionID == 1:
            return "Pangaea Rules"

        return u""
	
def getNumCustomMapOptionValues(argsList):
        """
        Number of different choices for a particular setting
        argsList[0] is Option ID (int)
        Return an integer
        """
        optionID = argsList[0]
        if optionID == 0:
            return 2
        elif optionID == 1:
            return 2
        return 0
	
def getCustomMapOptionDescAt(argsList):
	"""
	Returns name of value of option at specified row
	argsList[0] is Option ID (int)
	argsList[1] is Selection Value ID (int)
	Return a Unicode string
	"""
	optionID = argsList[0]
	selectionID = argsList[1]
	if optionID == 0:
		if selectionID == 0:
			return "Start in Old World (Default)"
		elif selectionID == 1:
			return "Start Anywhere"
	elif optionID == 1:
		if selectionID == 0:
			return "Break Pangaeas (Default)"
		elif selectionID == 1:
			return "Allow Pangaeas"
	return u""
	
def getCustomMapOptionDefault(argsList):
	"""
	Returns default value of specified option
	argsList[0] is Option ID (int)
	Return an integer
	"""
	#Always zero in this case
	return 0
    
def isRandomCustomMapOption(argsList):
	"""
	Returns a flag indicating whether a random option should be provided
	argsList[0] is Option ID (int)
	Return a bool
	"""
	return False
    
def getTopLatitude():
	"Default is 90. 75 is past the Arctic Circle"
	return 80

def getBottomLatitude():
	"Default is -90. -75 is past the Antartic Circle"
	return -80
def getGridSize(argsList):
	"Adjust grid sizes for optimum results"
	grid_sizes = {
		WorldSizeTypes.WORLDSIZE_DUEL:		(12,8),
		WorldSizeTypes.WORLDSIZE_TINY:		(16,10),
		WorldSizeTypes.WORLDSIZE_SMALL:		(22,14),
		WorldSizeTypes.WORLDSIZE_STANDARD:	(26,16),
		WorldSizeTypes.WORLDSIZE_LARGE:		(32,20),
		WorldSizeTypes.WORLDSIZE_HUGE:		(36,24)
	}
	if (argsList[0] == -1): # (-1,) is passed to function on loads
		return []
	[eWorldSize] = argsList
	return grid_sizes[eWorldSize]

def generatePlotTypes():
    gc = CyGlobalContext()
    mmap = gc.getMap()
    iNumPlotsX = mmap.getGridWidth()
    iNumPlotsY = mmap.getGridHeight()
    mc.initialize()
    mc.initInGameOptions()
    PWRand.seed()
    hm.GenerateMap(iNumPlotsX,iNumPlotsY,mc.LandPercent)
    plotTypes = [PlotTypes.PLOT_OCEAN] * (iNumPlotsX*iNumPlotsY)

    for i in range(iNumPlotsX*iNumPlotsY):
        mapLoc = hm.plotMap[i]
        if mapLoc == hm.PEAK:
            plotTypes[i] = PlotTypes.PLOT_PEAK
        elif mapLoc == hm.HILLS:
            plotTypes[i] = PlotTypes.PLOT_HILLS
        elif mapLoc == hm.LAND:
            plotTypes[i] = PlotTypes.PLOT_LAND
        else:
            plotTypes[i] = PlotTypes.PLOT_OCEAN
    print "Finished generating plot types."         
    return plotTypes
def generateTerrainTypes():
    NiTextOut("Generating Terrain  ...")
    print "Adding Terrain"
    gc = CyGlobalContext()
    terrainDesert = gc.getInfoTypeForString("TERRAIN_DESERT")
    terrainPlains = gc.getInfoTypeForString("TERRAIN_PLAINS")
    terrainIce = gc.getInfoTypeForString("TERRAIN_SNOW")
    terrainTundra = gc.getInfoTypeForString("TERRAIN_TUNDRA")
    terrainGrass = gc.getInfoTypeForString("TERRAIN_GRASS")
    terrainHill = gc.getInfoTypeForString("TERRAIN_HILL")
    terrainCoast = gc.getInfoTypeForString("TERRAIN_COAST")
    terrainOcean = gc.getInfoTypeForString("TERRAIN_OCEAN")
    terrainPeak = gc.getInfoTypeForString("TERRAIN_PEAK")
# Rise of Mankind start 2.5
    terrainMarsh = gc.getInfoTypeForString("TERRAIN_MARSH")
# Rise of Mankind end 2.5
    
    tm.GenerateTempMap(80,-80)
    rm.GenerateRainMap(80,-80)
    terr.GenerateTerrainMap()
    terr.generateContinentMap()   
    terrainTypes = [0]*(hm.mapWidth*hm.mapHeight)
    for i in range(hm.mapWidth*hm.mapHeight):
        if terr.terrainMap[i] == terr.OCEAN:
            terrainTypes[i] = terrainOcean
        elif terr.terrainMap[i] == terr.COAST:
            terrainTypes[i] = terrainCoast
        elif terr.terrainMap[i] == terr.DESERT:
            terrainTypes[i] = terrainDesert
        elif terr.terrainMap[i] == terr.PLAINS:
            terrainTypes[i] = terrainPlains
        elif terr.terrainMap[i] == terr.GRASS:
            terrainTypes[i] = terrainGrass
# Rise of Mankind start 2.5			
        elif terr.terrainMap[i] == terr.MARSH:
            terrainTypes[i] = terrainMarsh
# Rise of Mankind end 2.5			
        elif terr.terrainMap[i] == terr.TUNDRA:
            terrainTypes[i] = terrainTundra
        elif terr.terrainMap[i] == terr.ICE:
            terrainTypes[i] = terrainIce
    print "Finished generating terrain types."
    return terrainTypes
def doRiver(x,y,direction,riverID,riverLength):
    gc = CyGlobalContext()
    pmap = gc.getMap()
    startPlot = pmap.plot(x,y)
    riverPlot = 0
    adjPlot = 0
    if startPlot.getRiverID() == -1:
        startPlot.setRiverID(riverID)
    #meeting another river is the end of this river
    elif startPlot.getRiverID() != riverID:
        return riverLength
    if direction == rivers.N:
        riverPlot = startPlot
        adjPlot = pmap.plot(x+1,y)#East
        if adjPlot == 0:
            return riverLength
        if adjPlot.isWater() == True or riverPlot.isWater() == True or \
        riverPlot.isWOfRiver == True:
            return riverLength
        startPlot.setRiverID(riverID)
        riverPlot.setWOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_NORTH)
        riverPlot = pmap.plot(x,y + 1)#North
    elif direction == rivers.E:
        riverPlot = pmap.plot(x+1,y)#East
        if riverPlot == 0:
            return riverLength
        adjPlot = pmap.plot(x+1,y-1)#SouthEast
        if adjPlot == 0:
            return riverLength
        if adjPlot.isWater() == True or riverPlot.isWater() == True or \
        riverPlot.isNOfRiver == True:
            return riverLength
        startPlot.setRiverID(riverID)
        riverPlot.setNOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_EAST)
    elif direction == rivers.S:
        riverPlot = pmap.plot(x,y-1)#South
        if riverPlot == 0:
            return riverLength
        adjPlot = pmap.plot(x+1,y-1)#SouthEast
        if adjPlot == 0:
            return riverLength
        if riverPlot.isWOfRiver() == True or adjPlot.isWater() == True or \
        riverPlot.isWater() == True:
            return riverLength
        startPlot.setRiverID(riverID)
        riverPlot.setWOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_SOUTH)
    elif direction == rivers.W:
        riverPlot = startPlot
        adjPlot = pmap.plot(x,y-1)#South
        if adjPlot == 0:
            return riverLength
        if riverPlot.isNOfRiver == True or adjPlot.isWater() == True or \
        riverPlot.isWater() == True:
            return riverLength
        startPlot.setRiverID(riverID)
        riverPlot.setNOfRiver(True,CardinalDirectionTypes.CARDINALDIRECTION_WEST)
        riverPlot = pmap.plot(x - 1,y)#West
        
    if riverPlot == 0:
        return riverLength#River off the edge of map
    #Get next direction rivers.riverMap should cooincide with the SE corner of plot
    x = riverPlot.getX()
    y = riverPlot.getY()
    sePlot = pmap.plot(x+1,y-1)#SE
    if sePlot != 0 and sePlot.isWater() == True:
        return riverLength#River into the ocean
    i = hm.GetIndex(x,y)
    direction = rivers.riverMap[i]
    if direction != rivers.L or direction != rivers.O:
        riverLength = doRiver(x,y,direction,riverID,riverLength + 1)
    return riverLength

riverList = list()        
def addRivers():
    NiTextOut("Adding Rivers....")
    print "Adding Rivers"
    gc = CyGlobalContext()
    pmap = gc.getMap()
    nextRiverID = 0
    rivers.GenerateRiverMap()
##    PrintRiverAndTerrainAlign()
    for y in range(hm.mapHeight):
        for x in range(hm.mapWidth):
            i = hm.GetIndex(x,y)
            #Check for a river source that has a direction and also that
            #has 4 cardinal neighbors that have no river or a river that
            #leads away
            source = True #until proven wrong
            if rivers.riverMap[i] != rivers.O and rivers.riverMap[i] != rivers.L:
                xx = x
                yy = y + 1
                ii = hm.GetIndex(xx,yy)
                if rivers.riverMap[ii] == rivers.S:
                    source = False
                xx = x
                yy = y - 1
                ii = hm.GetIndex(xx,yy)
                if rivers.riverMap[ii] == rivers.N:
                    source = False
                xx = x - 1
                yy = y 
                ii = hm.GetIndex(xx,yy)
                if rivers.riverMap[ii] == rivers.E:
                    source = False
                xx = x + 1
                yy = y
                ii = hm.GetIndex(xx,yy)
                if rivers.riverMap[ii] == rivers.W:
                    source = False
            else:
                source = False
            if source == True:
                riverLength = doRiver(x,y,rivers.riverMap[i],nextRiverID,0)
                riverList.append(riverLength)
                nextRiverID += 1

    #peaks and rivers don't always mix well graphically, so lets eliminate
    #these potential glitches. Basically if there are adjacent peaks on both
    #sides of a river, either in a cardinal direction or diagonally, they
    #will look bad.
    for y in range(hm.mapHeight):
        for x in range(hm.mapWidth):
            plot = pmap.plot(x,y)
            if plot.isPeak() == True:
                if plot.isNOfRiver() == True:
                    for xx in range(x - 1,x + 2,1):
                        yy = y - 1
                        if yy < 0:
                            break
                        #wrap in x direction
                        if xx == -1:
                            xx = hm.mapWidth - 1
                        elif xx == hm.mapWidth:
                            xx = 0
                        newPlot = pmap.plot(xx,yy)
                        if newPlot.isPeak():
                            plot.setPlotType(PlotTypes.PLOT_HILLS,True,True)
                            break
            #possibly changed so checked again
            if plot.isPeak() == True:
                if plot.isWOfRiver() == True:
                    for yy in range(y - 1,y + 2,1):
                        xx = x + 1
                        if xx == hm.mapWidth:
                            xx = 0
                        #do not wrap in y direction
                        if yy == -1:
                            continue
                        elif yy == hm.mapHeight:
                            continue
                        newPlot = pmap.plot(xx,yy)
                        if newPlot.isPeak():
                            plot.setPlotType(PlotTypes.PLOT_HILLS,True,True)
                            break

    print nextRiverID," rivers generated."

def addFeatures():
    NiTextOut("Generating Features  ...")
    print "Adding Features"
    gc = CyGlobalContext()
    mmap = gc.getMap()
    featureIce = gc.getInfoTypeForString("FEATURE_ICE")
    featureJungle = gc.getInfoTypeForString("FEATURE_JUNGLE")
    featureOasis = gc.getInfoTypeForString("FEATURE_OASIS")
    featureFloodPlains = gc.getInfoTypeForString("FEATURE_FLOOD_PLAINS")
    featureForest = gc.getInfoTypeForString("FEATURE_FOREST")
# Rise of Mankind start 2.5
    featureSwamp = gc.getInfoTypeForString("FEATURE_SWAMP")
# Rise of Mankind end 2.5
    FORESTLEAFY = 0
    FORESTEVERGREEN = 1
    FORESTSNOWY = 2
    #First create ice. We want less ice than the default map generator.
    iceChance = 1.0
    for y in range(4):
        for x in range(hm.mapWidth):
            plot = mmap.plot(x,y)
            if plot != 0 and plot.isWater() == True and PWRand.random() < iceChance:
                plot.setFeatureType(featureIce,0)
        iceChance *= .66
    iceChance = 1.0
    for y in range(hm.mapHeight - 1,hm.mapHeight - 5,-1):
        for x in range(hm.mapWidth):
            plot = mmap.plot(x,y)
            if plot != 0 and plot.isWater() == True and PWRand.random() < iceChance:
                plot.setFeatureType(featureIce,0)
        iceChance *= .66
    #Now plant forest or jungle and place floodplains and oasis
##    PrintTempMap(tm,tm.tempMap)
##    PrintRainMap(rm,rm.rainMap,False)
    for y in range(hm.mapHeight):
        for x in range(hm.mapWidth):
            i = hm.GetIndex(x,y)
            plot = mmap.plot(x,y)
            #forest and jungle
            if plot.isWater() == False and terr.terrainMap[i] != terr.DESERT and\
            hm.plotMap[i] != hm.PEAK:
                if rm.rainMap[i] > terr.plainsThreshold*1.5:#jungle
                    if tm.tempMap[i] > mc.JungleTemp:
                        if terr.terrainMap[i] == terr.PLAINS:
                            plot.setFeatureType(featureForest,FORESTLEAFY)
                        else:
                            plot.setFeatureType(featureJungle,0)
                    elif tm.tempMap[i] > mc.ForestTemp:
                        plot.setFeatureType(featureForest,FORESTLEAFY)
                    elif tm.tempMap[i] > mc.TundraTemp:
                        plot.setFeatureType(featureForest,FORESTEVERGREEN)
                    elif tm.tempMap[i] > mc.IceTemp:
                        plot.setFeatureType(featureForest,FORESTSNOWY)
                elif rm.rainMap[i] > terr.desertThreshold:#forest
                    if rm.rainMap[i] > PWRand.random() * terr.plainsThreshold * 1.5:
                        if tm.tempMap[i] > mc.ForestTemp:
                           plot.setFeatureType(featureForest,FORESTLEAFY)
                        elif tm.tempMap[i] > mc.TundraTemp:
                            plot.setFeatureType(featureForest,FORESTEVERGREEN)
                        elif tm.tempMap[i] > mc.IceTemp * 0.8:
                            plot.setFeatureType(featureForest,FORESTSNOWY)
            #floodplains and Oasis
            elif terr.terrainMap[i] == terr.DESERT and hm.plotMap[i] != hm.HILLS and\
            hm.plotMap[i] != hm.PEAK and plot.isWater() == False:
                if plot.isRiver() == True and \
                rivers.isFloodPlainThreshold(x,y) == True:
                    plot.setFeatureType(featureFloodPlains,0)
                else:
                    #is this square surrounded by desert?
                    foundNonDesert = False
                    #print "trying to place oasis"
                    for yy in range(y - 1,y + 2):
                        for xx in range(x - 1,x + 2):
                            ii = hm.GetIndex(xx,yy)
                            surPlot = mmap.plot(xx,yy)
                            if (terr.terrainMap[ii] != terr.DESERT and \
                            hm.plotMap[ii] != hm.PEAK):
                                #print "non desert neighbor"
                                foundNonDesert = True
                            elif surPlot == 0:
                                #print "neighbor off map"
                                foundNonDesert = True
                            elif surPlot.isWater() == True:
                                #print "water neighbor"
                                foundNonDesert = True
                            elif surPlot.getFeatureType() == featureOasis:
                                #print "oasis neighbor"
                                foundNonDesert = True
                    if foundNonDesert == False:
                        if PWRand.random() < mc.OasisChance:
                            #print "placing oasis"
                            plot.setFeatureType(featureOasis,0)
                       # else:
                            #print "oasis failed random check"
            
    return
def addBonuses():
    bp.AddBonuses()
    return
def assignStartingPlots():
    gc = CyGlobalContext()
    gameMap = CyMap()
    iPlayers = gc.getGame().countCivPlayersEverAlive()
    spf.SetStartingPlots()


##############################################################################
#debugging stuff
##############################################################################
def PrintWindDirection(we,ns):
    dString = "-"
    if(ns == 1):
        dString += '^'
    else:
        dString += 'v'
    if(we == 1):
        dString += '>'
    else:
        dString += '<'
    return dString        
    
def PrintDistMap(distMap,distWidth,distHeight):
    print "Local Distribution Map"
    for y in range(distHeight - 1,-1,-1):
        lineString = ""
        for x in range(distWidth):
            mapLoc = distMap[y * distWidth + x]
            lineString += '%(mapLoc)0.2f ' % {'mapLoc' : mapLoc}
        print lineString

    lineString = " "
    print lineString
    return
def PrintHeightMap(hm):
    print "Height Map"
    for y in range(hm.mapHeight):
        lineString = ""
        for x in range(hm.mapWidth):
            mapLoc = hm.map[hm.GetIndex(x,y)]
            if mapLoc > hm.hillLevel:
                lineString += 'P'
            elif mapLoc > hm.flatLevel:
                lineString += 'H'
            elif mapLoc > hm.seaLevel:
                lineString += 'F'
            else:
                lineString += 'O'
        print lineString
    lineString = " "
    print lineString

    return
def PrintPlateMap(hm):
    print "Plate Map"
    for y in range(hm.mapHeight):
        lineString = ""
        for x in range(hm.mapWidth):
            mapLoc = hm.plateMap[hm.GetIndex(x,y)]
            lineString += chr(mapLoc + 33)
        print lineString
    lineString = " "
    print lineString

    return
def PrintInfluMap(hm):
    print "Influence Map"
    for y in range(hm.mapHeight):
        lineString = ""
        for x in range(hm.mapWidth):
            mapLoc = hm.influMap[hm.GetIndex(x,y)]
            lineString += chr(int(mapLoc*10) - 8 + 48)
        print lineString

    lineString = " "
    print lineString
    return
def PrintPlotMap(hm):
    print "Plot Map"
    for y in range(hm.mapHeight - 1,-1,-1):
        lineString = ""
        for x in range(hm.mapWidth):
            mapLoc = hm.plotMap[hm.GetIndex(x,y)]
            if mapLoc == hm.PEAK:
                lineString += 'P'
            elif mapLoc == hm.HILLS:
                lineString += 'H'
            elif mapLoc == hm.LAND:
                lineString += 'F'
            else:
                lineString += '.'
        print lineString
    lineString = " "
    print lineString

    return
def PrintTempMap(tm,tMap):
    print "Temperature Map"
    for y in range(hm.mapHeight - 1,-1,-1):
        lineString = ""
        for x in range(hm.mapWidth):
            mapLoc = tMap[hm.GetIndex(x,y)]
            i = hm.GetIndex(x,y)
            if hm.plotMap[i] == hm.OCEAN:
                lineString += '.'
            else:
                lineString += chr(int(mapLoc*10) + 48)
        lineString += "-" + rm.wz.GetZoneName(tm.wz.GetZone(y))
        print lineString

    lineString = " "
    print lineString
    return
def PrintRainMap(rm,rMap,bOcean):
    print "Rain Map"
    for y in range(hm.mapHeight - 1,-1,-1):
        lineString = ""
        for x in range(hm.mapWidth):
            if bOcean:
                if hm.plotMap[hm.GetIndex(x,y)] == hm.OCEAN:
                    mapLoc = rMap[hm.GetIndex(x,y)]
                    lineString += chr(int(mapLoc*10) + 48)
                else:
                    lineString += 'X'
            else:
                if hm.plotMap[hm.GetIndex(x,y)] == hm.OCEAN:
                    lineString += '.'
                else:
                    mapLoc = rMap[hm.GetIndex(x,y)]
                    lineString += chr(int(mapLoc*10) + 48)
        we,ns = rm.wz.GetWindDirectionsInZone(rm.wz.GetZone(y))
        lineString += "-" + rm.wz.GetZoneName(rm.wz.GetZone(y)) + PrintWindDirection(we,ns)
        print lineString

    lineString = " "
    print lineString
    return
def PrintTruncRainMap(rm,rMap,bOcean):
    print "Truncated Rain Map"
    minV = 1.0
    for i in range(hm.mapWidth*hm.mapHeight):
        if rMap[i] > 0.0 and rMap[i] < minV:
            minV = rMap[i]
    print "min value",minV
    for y in range(hm.mapHeight - 1,-1,-1):
        lineString = ""
        for x in range(hm.mapWidth):
            if bOcean:
                if hm.plotMap[hm.GetIndex(x,y)] == hm.OCEAN:
                    mapLoc = rMap[hm.GetIndex(x,y)]
                    lineString += chr(int(mapLoc*10) + 48)
                else:
                    lineString += 'X'
            else:
                if hm.plotMap[hm.GetIndex(x,y)] == hm.OCEAN:
                    lineString += '.'
                else:
                    mapLoc = rMap[hm.GetIndex(x,y)]
                    if mapLoc == minV:
                        lineString += 'X'
                    else:    
                        mapLoc *= 10
                        if mapLoc > .95:
                            mapLoc = .95
                        lineString += chr(int(mapLoc*10) + 48)
        we,ns = rm.wz.GetWindDirectionsInZone(rm.wz.GetZone(y))
        lineString += "-" + rm.wz.GetZoneName(rm.wz.GetZone(y)) + PrintWindDirection(we,ns)
        print lineString

    lineString = " "
    print lineString
    return
def PrintTerrainMap():
    print "Terrain Map"
    for y in range(hm.mapHeight - 1,-1,-1):
        lineString = ""
        for x in range(hm.mapWidth):
            mapLoc = terr.terrainMap[hm.GetIndex(x,y)]
            if mapLoc == terr.OCEAN:
                lineString += '.'
            elif mapLoc == terr.COAST:
                lineString += '.'
            elif mapLoc == terr.DESERT:
                lineString += 'D'
            elif mapLoc == terr.GRASS:
                lineString += 'R'
            elif mapLoc == terr.PLAINS:
                lineString += 'P'
            elif mapLoc == terr.TUNDRA:
                lineString += 'T'
            elif mapLoc == terr.ICE:
                lineString += 'I'
        lineString += "-" + rm.wz.GetZoneName(rm.wz.GetZone(y))
        print lineString
    lineString = " "
    print lineString
        
def PrintRiverMap():
    print "River Map"
    for y in range(hm.mapHeight - 1,-1,-1):
        lineString = ""
        for x in range(hm.mapWidth):
            mapLoc = rivers.riverMap[hm.GetIndex(x,y)]
            if mapLoc == rivers.O:
                lineString += '.'
            elif mapLoc == rivers.L:
                lineString += 'L'
            elif mapLoc == rivers.N:
                lineString += 'N'
            elif mapLoc == rivers.S:
                lineString += 'S'
            elif mapLoc == rivers.E:
                lineString += 'E'
            elif mapLoc == rivers.W:
                lineString += 'W'
        lineString += "-" + rm.wz.GetZoneName(rm.wz.GetZone(y))
        print lineString
    lineString = " "
    print lineString
        
def PrintFlowMap():
    print "Flow Map"
    for y in range(hm.mapHeight - 1,-1,-1):
        lineString = ""
        for x in range(hm.mapWidth):
            mapLoc = rivers.flowMap[hm.GetIndex(x,y)]
            if mapLoc == rivers.O:
                lineString += '.'
            elif mapLoc == rivers.L:
                lineString += 'L'
            elif mapLoc == rivers.N:
                lineString += 'N'
            elif mapLoc == rivers.S:
                lineString += 'S'
            elif mapLoc == rivers.E:
                lineString += 'E'
            elif mapLoc == rivers.W:
                lineString += 'W'
        lineString += "-" + rm.wz.GetZoneName(rm.wz.GetZone(y))
        print lineString
    lineString = " "
    print lineString
def PrintRiverAndTerrainAlign():
    print "River Alignment Check"
    for y in range(hm.mapHeight - 1,-1,-1):
        lineString1 = ""
        lineString2 = ""
        for x in range(hm.mapWidth):
            mapLoc = terr.terrainMap[hm.GetIndex(x,y)]
            if mapLoc == terr.OCEAN:
                lineString1 += 'O.'
            elif mapLoc == terr.COAST:
                lineString1 += 'O.'
            elif mapLoc == terr.DESERT:
                lineString1 += 'D.'
            elif mapLoc == terr.GRASS:
                lineString1 += 'R.'
            elif mapLoc == terr.PLAINS:
                lineString1 += 'P.'
            elif mapLoc == terr.TUNDRA:
                lineString1 += 'T.'
            elif mapLoc == terr.ICE:
                lineString1 += 'I.'
            mapLoc = rivers.riverMap[hm.GetIndex(x,y)]
            if mapLoc == rivers.O:
                lineString2 += '..'
            elif mapLoc == rivers.L:
                lineString2 += '.L'
            elif mapLoc == rivers.N:
                lineString2 += '.N'
            elif mapLoc == rivers.S:
                lineString2 += '.S'
            elif mapLoc == rivers.E:
                lineString2 += '.E'
            elif mapLoc == rivers.W:
                lineString2 += '.W'
        lineString1 += "-" + rm.wz.GetZoneName(rm.wz.GetZone(y))
        lineString2 += "-" + rm.wz.GetZoneName(rm.wz.GetZone(y))
        print lineString1
        print lineString2
    lineString1 = " "
    print lineString1
     
def PrintAvgRainMap():
    print "Avg Rain Map"
    for y in range(hm.mapHeight):
        lineString = ""
        for x in range(hm.mapWidth):
                if hm.plotMap[hm.GetIndex(x,y)] == hm.OCEAN:
                    lineString += '.'
                else:
                    mapLoc = rivers.averageRainfallMap[hm.GetIndex(x,y)]
                    lineString += chr(int(mapLoc*10) + 48)
        lineString += "-" + rm.wz.GetZoneName(rm.wz.GetZone(y))
        print lineString

    lineString = " "
    print lineString
    return

##debugFile = open("debugout.txt","w")
##standardFile = sys.__stdout__
##sys.stdout = debugFile
##hm.GenerateMap(144,96,LandPercent)
##tm.GenerateTempMap(75,-75)
##rm.GenerateRainMap(75,-75)
##terr.GenerateTerrainMap()
##terr.generateContinentMap()
##rivers.GenerateRiverMap()
##PrintPlotMap(hm)
##PrintTempMap(tm,tm.sunMap)
##PrintTempMap(tm,tm.tempMap)
##PrintRainMap(rm,rm.rainMap,False)
##PrintTruncRainMap(rm,rm.rainMap,False)
##PrintTerrainMap()
##areaTest = Areamap(hm.mapWidth,hm.mapHeight)
##areaTest.defineAreas(False)
##areaTest.PrintAreaMap()
##areaTest.PrintList(areaTest.areaList)
##PrintFlowMap()
##PrintRiverMap()
##PrintRiverAndTerrainAlign()
##debugFile.close()
##sys.stdout = standardFile




