/*=========================================================================

  Program:   ITKXML
  Module:    kwsGenerator.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Author:    Julien Jomier

  Copyright (c) 2002 CADDLab @ UNC. All rights reserved.
  See itkUNCCopyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsGenerator.h"

namespace kws {

/** Constructor */
Generator::Generator()
{
  m_Parsers = NULL;
}

/** Destructor */
Generator::~Generator()
{
}

/** Generate the Matrix representation */
bool Generator::GenerateMatrix(const char* dir)
{
  std::cout << "Generating Matrix...";
  std::ofstream file;
  file.open("KWSMatrix.html", std::ios::binary | std::ios::out);
  if(!file.is_open())
    {
    std::cout << "Cannot open file for writing: " <<  std::endl;
    return false;
    }
    
  this->CreateHeader(&file,"KWSMatrix.html");


  // Contruct the table
  file << "<table width=\"100%\" border=\"0\" height=\"1\">" << std::endl;
  file << "<tr>" << std::endl;
  file << "  <td width=\"30%\"> " << std::endl;
  file << "    <div align=\"center\">Filename</div>" << std::endl;
  file << "  </td>" << std::endl;

  unsigned int width = 70/NUMBER_ERRORS;
  for(unsigned int i=0;i<NUMBER_ERRORS;i++)
    {
    file << "  <td width=\"" << width << "%\">" << std::endl;
    file << "    <div align=\"center\">" << ErrorTag[i] << "</div>" << std::endl;
    file << "  </td>" << std::endl;
    }
  file << "</tr>" << std::endl;

  ParserVectorType::const_iterator it = m_Parsers->begin();
  while(it != m_Parsers->end())
    {
    // Extract the filename
    std::string filename = dir;
    filename += "/";
    long int slash = (*it).GetFilename().find_last_of("/");
    if(slash == -1)
      {
      slash = 0;
      }
    std::string nameofclass = (*it).GetFilename().substr(slash+1,((*it).GetFilename().size())-slash-1);
    filename += nameofclass;
    std::string filenamecorrect = nameofclass;
    filename += ".html";

    // Fill in the table
    file << "<tr>" << std::endl;
    file << "  <td width=\"30%\"> " << std::endl;
    file << "    <div align=\"center\"> <a href=\"" << filename.c_str()  << "\">" << filenamecorrect.c_str() << "</a></div>" << std::endl;
    file << "  </td>" << std::endl;

    unsigned int width = 50/NUMBER_ERRORS;
    for(unsigned int i=0;i<NUMBER_ERRORS;i++)
      {
      // Count the number of errors for this type of error
      unsigned int nerror = 0;
      
      Parser::ErrorVectorType errors = (*it).GetErrors();
      Parser::ErrorVectorType::const_iterator itError = errors.begin();
      while(itError != errors.end())
        {
        if((*itError).number == i+1)
          {
          nerror++;
          }
        itError++;
        }


      file << "  <td width=\"" << width << "%\"";
      if(nerror == 0)
        {
        if(!(*it).HasBeenPerformed(i+1))
          {
          file << "bgcolor=\"#cccccc\"";
          }
        else
          {
          file << "bgcolor=\"#00aa00\"";
          }
        }
      else
        {
        file << "bgcolor=\"#ff6666\"";
        }
      file << ">" << std::endl;
      file << "    <div align=\"center\">" << nerror << "</div>" << std::endl;
      file << "  </td>" << std::endl;
      }
    file << "</tr>" << std::endl; 

    it++;
    }
   

  file << "</table>" << std::endl;
  
  this->CreateFooter(&file);    
  file.close();

  std::cout << "done" << std::endl;
  return true;
}

/** Generate the HTML files */
bool Generator::GenerateHTML(const char* dir)
{
  // Generate the matrix representation
  this->GenerateMatrix(dir);

  std::cout << "Generating HTML...";

  // For each file we generate an html page
  ParserVectorType::const_iterator it = m_Parsers->begin();
  while(it != m_Parsers->end())
    {
    std::ofstream file;

    if((*it).GetFilename().size() == 0)
      {
      it++;
      continue;
      }

    // Extract the filename
    std::string filename = dir;
    filename += "/";
    long int slash = (*it).GetFilename().find_last_of("/");
    if(slash == -1)
      {
      slash = 0;
      }
    std::string nameofclass = (*it).GetFilename().substr(slash+1,((*it).GetFilename().size())-slash-1);
  
    filename += nameofclass;
    filename += ".html";

    //std::cout << " " << filename.c_str() << " ";

    file.open(filename.c_str(), std::ios::binary | std::ios::out);
    if(!file.is_open())
      {
      std::cout << "Cannot open file for writing: " <<  std::endl;
      return false;
      }

    this->CreateHeader(&file,filename.c_str());

    file << "<table width=\"100%\" border=\"0\" height=\"1\">" << std::endl;
  
    bool comment = false;
    for(unsigned int i=0;i<(*it).GetNumberOfLines();i++)
      {
      
      // Look in the errors if there is a match for this line
      int error = 0;
      std::string errorTag = "";

      const Parser::ErrorVectorType errors = (*it).GetErrors();
      Parser::ErrorVectorType::const_iterator itError = errors.begin();
      while(itError != errors.end())
        {
        if( ((i+1>=(*itError).line) && (i+1<=(*itError).line2))
          )
          {
          if(errorTag.size() == 0)
            {
            errorTag += (*it).GetErrorTag((*itError).number);
            }
          else
            {
            errorTag += ",";
            errorTag += (*it).GetErrorTag((*itError).number);
            }
          error = (*itError).number;
          }
        itError++;
        }

      if(error)
        {
        file << "<tr bgcolor=\"" << ErrorColor[error-1]  << "\">" << std::endl; 
        }
      else
        {
        file << "<tr>" << std::endl;
        }
      
      // First column is the line number
      file << "<td height=\"1\">" << i+1 << "</td>" << std::endl;
      
      // Second column is the error tag
      file << "<td height=\"1\">" << errorTag.c_str() << "</td>" << std::endl;
      
      std::string l = (*it).GetLine(i);

      // If the error is of type INDENT we show the problem as _
      if(errorTag.find("IND") != -1)
        {
        unsigned int k = 0;
        while((l[k] == ' ') || (l[k] == '\n'))
          {
          if(l[k] == ' ')
            {
            l[k]='*'; 
            }
          k++;
          }
        }

      // Remove the first \n
      long int p = l.find('\n');
      if(p != -1)
        {
        l.replace(p,1,"");
        }
      
      // Replace < and >
      long int inf = l.find("<",0);
      while(inf != -1)
        {
        l.replace(inf,1,"&lt;");
        inf = l.find("<",0);
        }

      long int sup = l.find(">",0);
      while(sup != -1)
        {
        l.replace(sup,1,"&gt;");
        sup = l.find(">",0);
        }

      // Replace the space by &nbsp;
      long int space = l.find(' ',0);
      while(space != -1)  
        {
        l.replace(space,1,"&nbsp;");
        space = l.find(' ',space+1);
        }

      // Show the comments in green
      if(comment)
        {
        l.insert(0,"<font color=\"#009933\">");
        }

      space = l.find("/*",0);
      while(space != -1)  
        {
        comment = true;
        l.insert(space,"<font color=\"#009933\">");
        space = l.find("/*",space+23);
        }

      if(comment)
        {
        l.insert(l.size(),"</font>");
        }

      space = l.find("*/",0);
      while(space != -1)
        {
        comment = false;
        l.insert(space+2,"</font>");
        space = l.find("*/",space+8);
        }

      // Show the comments in green
      space = l.find("//",0);
      if(space != -1)  
        {
        l.insert(space,"<font color=\"#009933\">");
        l += "<font>";
        }

      file << "<td height=\"1\"><font face=\"Courier New, Courier, mono\" size=\"2\">" << l.c_str() << "</font></td>" << std::endl;
      file << "</tr>" << std::endl;
      }

    file << "</table>" << std::endl;

    this->CreateFooter(&file);
    
    file.close();
    it++;
    }

  std::cout << "done" << std::endl;
  return true;
}

/** Create Header */
bool Generator::CreateHeader(std::ofstream * file,const char* title)
{
  *file << "<html>" << std::endl;
  *file << "<HEAD>" << std::endl;
  *file << "<META name=\"description\" content=\"kitware.com\">" << std::endl;
  *file << "<META name=\"keywords\" content=\"kwstyle,Kitware,Style,Checker,Dart\">" << std::endl;
  *file << "<META name=\"author\" content=\"Kitware\">" << std::endl;
  *file << "<META name=\"revisit-after\" content=\"2 days\">" << std::endl;
  *file << "<META name=\"robots\" content=\"all\">" << std::endl;
  *file << "<title>KWStyle - " << title << "</title>" << std::endl;
  *file << "</HEAD>" << std::endl;

  // Now create the top frame
 *file << "<table width=\"100%\" border=\"0\">" << std::endl;
 *file << " <tr>" << std::endl;
 *file << "   <td width=\"15%\" height=\"2\"><img src=\"Logo.gif\" width=\"100\" height=\"64\"></td>" << std::endl;
 *file << "   <td width=\"85%\" height=\"2\" bgcolor=\"#0099CC\"> " << std::endl;

 // remove the last extension
 std::string tit = title;
 long int pos = tit.find_last_of(".");
 if(pos!=-1)
   {
   tit = tit.substr(0,pos);
   }
 pos = tit.find_last_of("/");
 if(pos!=-1)
   {
   tit = tit.substr(pos+1,tit.size()-pos-1);
   }
 
 *file << "     <div align=\"left\"><b><font color=\"#FFFFFF\" size=\"5\">KWStyle - " << tit.c_str() << "</font></b></div>" << std::endl;
 *file << "   </td>" << std::endl;
 *file << " </tr>" << std::endl;
 *file << "</table>" << std::endl;
 *file << "<table width=\"100%\" border=\"0\">" << std::endl;
 *file << " <tr> " << std::endl;
 *file << "   <td width=\"15%\" height=\"30\" >&nbsp;</td>" << std::endl;
 *file << "   <td height=\"30\" width=\"12%\" bgcolor=\"#0099CC\"> " << std::endl;
 *file << "     <div align=\"center\"><a href=\"KWSMatrix.html\">Matrix View</a></div>" << std::endl;
 *file << "   </td>" << std::endl;
 *file << "   <td width=\"10%\" height=\"30\">&nbsp;</td>" << std::endl;
 *file << "   <td width=\"63%\" height=\"30\"> " << std::endl;
 *file << "     <div align=\"left\"><b></b></div>" << std::endl;
 *file << "     <div align=\"right\"></div>" << std::endl;
 *file << "   </td>" << std::endl;
 *file << " </tr>" << std::endl;
 *file << "</table>" << std::endl;
 *file << "<hr size=\"1\">";
  return true;
}

/** Create Footer */
bool Generator::CreateFooter(std::ofstream * file)
{
  *file << "<hr size=\"1\">";
  *file << "<table width=\"100%\" border=\"0\">";
  *file << "<tr>";
  *file << "<td>Generated by <a href=\"http://public.kitware.com/KWStyle\">KWStyle</a> 1.0b on <i>" << itksys::SystemTools::GetCurrentDateTime("%A %B,%d at %I:%M:%S%p") << "</i></td>";
  *file << "<td>";
  *file << "<div align=\"center\"><img src=\"logosmall.jpg\" width=\"160\" height=\"49\"></div>" << std::endl;
  *file << "</td>";
  *file << "<td>";
  *file << "<div align=\"right\"><a href=\"http://www.kitware.com\">&copy; Kitware Inc.</a></div></td>";
  *file << "</tr>";
  *file << "</table>";
  *file << "<br>";

  *file << "</html>" << std::endl;

  return true;
}

} // end namespace kws
