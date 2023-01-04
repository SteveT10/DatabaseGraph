
class Course:
    def __init__(self, category = "CATG", code = "CATG 101", credits = None, \
                days = "to be arranged", instructor = "to be arranged",\
                location = "to be arranged", sectID = 'N', sln = 12345,\
                 time = "to be arranged", title = "", quarter = "AUT2003"):
        self._category = category
        self._code = code
        self._credits = credits
        self._days = days 
        self._instructor = instructor
        self._location = location 
        self._sectionID = sectID
        self._sln = sln 
        self._time = time
        self._title = title
        self._quarter = quarter

    def __eq__(self, other):
        result = False
        if isinstance(other, Course):
            result = (self._quarter == other._quarter and \
                self._time == other._time and \
                self._days == other._days and \
                self._location == other._location and \
                self._title == other._title)
        return result

    def __repr__(self):
        return self._quarter + " " + self._code + self._sectionID + \
                 " " + self._title

    def __ne__(self, other):
        return (not self.__eq__(other))

    def __hash__(self):
        return hash((self._quarter, self._time, self._days, self._location, \
                     self._title))

    def isLab(self):
        return credits == "LB"

    def setCategory(self, category):
        self._category = category
    
    def setCode(self, code):
        self._code = code

    def setCredits(self, credits):
        self._credits = credits

    def setDays(self, days):
        self._days = days

    def setInstructor(self, name):
        self._instructor = name
    
    def setLocation(self, location):
        self._location = location

    def setSectionID(self, ID):
        self._sectionID = ID

    def setSLN(self, slNum):
        self._sln = slNum

    def setTime(self, time):
        self._time = time

    def setTitle(self, title):
        self._title = title

    def setQuarter(self, quarter):
        self._quarter = quarter

    def getCategory(self):
        return self._category

    def getCode(self):
        return self._code

    def getCredits(self):
        return self._credits
    
    def getDays(self):
        return self._days
    
    def getInstructor(self):
        return self._instructor

    def getLocation(self):
        return self._location
    
    def getSectionID(self):
        return self._sectionID

    def getSLN(self):
        return self._sln
    
    def getTime(self):
        return self._time

    def getTitle(self):
        return self._title

    def getQuarter(self):
        return self._quarter