import sqlite3
import datetime
import requests
import time
import os
from CourseObj import Course
from bs4 import BeautifulSoup
from bs4 import SoupStrainer
from collections import deque

def main():
    #Classes may have different codes but may be the same class to reserve spots:
    #We ignore the code and focus on make suring its the same class.
    #100 - 200 = freshman - sophomore level only
    #300 - 400 = juniors, seniors, fifth year level only
    #500 - 600 = graduate level only

    #No id or class used in elements, must find through other attributes
    currentDir = os.path.dirname(__file__)
    dbDir = os.path.relpath("..\\data\\TCEScourses.db", currentDir)
    print("\n" + dbDir)

    baseTag = "table" #all Information are stored in table elements
    dbBuffer = set()
    quarterQueue = deque([])
    dbConnection = sqlite3.connect(dbDir)
    dbCursor = dbConnection.cursor()
    createTbl = ("""
        CREATE TABLE IF NOT EXISTS courses(
            Category    TINYTEXT NOT NULL,
            SLN         VARCHAR(6),
            Section ID  VARCHAR(2),
            Credits     VARCHAR(4),
            Quarter     CHAR(7),
            Code        VARCHAR(15),
            Title       TINYTEXT,
            Days        TINYTEXT NOT NULL,
            Time        TINYTEXT NOT NULL,
            Location    TINYTEXT NOT NULL,
            Instructor  TINYTEXT NOT NULL);
    """)
    dbCursor.execute(createTbl)
    getMissingData(quarterQueue, dbCursor)
    print(quarterQueue)
    while len(quarterQueue) > 0:
        temp = quarterQueue.popleft()
        getPageData(dbBuffer, temp, baseTag)
        print(temp + "'s time schedule data recorded!\n")
        SendToDB(dbBuffer, dbCursor)
        dbConnection.commit()
        time.sleep(2)

def getMissingData(storageQueue, dbCursor):
    #TCES is available from AUT2007 to current.
    
    year = datetime.date.today().year + 1
    qtrCount = (year - 2007) * 4 
    qtrs = ["AUT", "WIN", "SPR", "SUM"]
    qtrDataExists = "SELECT EXISTS(SELECT Quarter FROM \
                courses WHERE courses.Quarter = '{}')"

    dbCursor.execute(qtrDataExists.format(qtrs[0] + str(year)))
    foundQtrData = dbCursor.fetchone()[0] #Array that contains 0 or 1.
    #print(foundQtrData)
    while qtrCount >= 0 and not foundQtrData: #AUT2007 is start, hence >=
        storageQueue.append(qtrs[qtrCount % 4] + str(year)) #append missing Qtr
        qtrCount -= 1
        if qtrCount % 4 == 0:
            year -= 1    
        dbCursor.execute(qtrDataExists.format(qtrs[qtrCount % 4] + str(year)))
        foundQtrData = dbCursor.fetchone()[0]

def getPageData(storageSet, pgQuarter, baseTag):

    baseURL = "https://www.washington.edu/students/timeschd/T/{}/tces.html"
    pageURL = baseURL.format(pgQuarter)

    webpage = requests.get(pageURL)
    parser = BeautifulSoup(webpage.content, "html.parser" ,\
                        parse_only = SoupStrainer(baseTag))
    for brElement in parser.find_all("br"):
        brElement.replace_with(" ")

    #large amount of space exists before final ")" character.
    #Note that text also contains \n and \r after spaces.
    if parser.find("h2") != None:
        currentCategory = parser.find("h2").text
        currentCategory = currentCategory[0 : currentCategory.index("  ")] + \
                        currentCategory[-1]

    coursesOnPage = parser.find_all(baseTag)
    currentTitle = ""
    currentCode = ""
    current = None
    courseDetails = ""

    for courseElement in coursesOnPage:

        #Colored Tables are a course title for the upcoming courses.
        if courseElement.get('bgcolor') != None and courseElement.get('bgcolor') != "#e0e0e0" and courseElement.get('bgcolor') != "#d3d3d3":

            currentCode = courseElement.a.string.replace(u'\xa0', "")
            #next_sibling was a non-breakable space, title comes after that.
            #print(courseElement.a.next_sibling)
            currentTitle = courseElement.a.next_sibling.next_sibling.string 
            #print(currentCode + currentTitle)

        elif currentTitle != "": #Is individual course entry with a known title
            
            courseDetails = ""
            for child in courseElement.pre.children: #Slower but handles all ele
                courseDetails += child.string

            courseDetails = courseDetails.split() #details Seperated by spaces.

            if not courseDetails[0].isalpha(): #courses with no restr are blank.
                courseDetails.insert(0, "No restr")
            print(courseDetails)
            #SLN = 1, SectID, Credit, Days, Time, Building, Room, Instructor.
            current = Course(
                category = currentCategory,
                quarter = pgQuarter,
                code = currentCode,
                title = currentTitle,
                sln = courseDetails[1],
                sectID = courseDetails[2],
                credits = courseDetails[3]
            )

            if courseDetails[4] == "to": #Still need to arrange details?
                storageSet.add(current)
                continue

            current.setDays(courseDetails[4])
            current.setTime(courseDetails[5])
            if courseDetails[6] == "*" and courseDetails[7] == "*":
                current.setLocation("Online")
            else:
                current.setLocation(courseDetails[6] + courseDetails[7])

            #Some are missing instructor only and we record wrong input.
            #To ensure that we are getting a name, there must a be
            #comma to seperate first and last name.
            if not courseDetails[8].isalpha():
                current.setInstructor(courseDetails[8])
            
            storageSet.add(current)

def SendToDB(bufferSet, dbCursor):
    insertCommand = ("""
        INSERT INTO courses VALUES
            ('{category}', '{sln}', '{sectID}', '{credits}', '{quarter}', '{code}',
			'{title}', '{days}', '{time}', '{location}', '{instructor}')
    """)
    for entry in bufferSet:
        dbCursor.execute(insertCommand.format(
            category = entry.getCategory(),
            sln = entry.getSLN(),
            sectID = entry.getSectionID(),
            credits = entry.getCredits(),
            quarter = entry.getQuarter(),
            code = entry.getCode(),
            title = entry.getTitle(),
            days = entry.getDays(),
            time = entry.getTime(),
            location = entry.getLocation(),
            instructor = entry.getInstructor()
        ))
    bufferSet.clear()

if __name__ == "__main__":
    main()
