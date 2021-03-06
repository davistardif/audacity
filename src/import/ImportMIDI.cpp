/**********************************************************************

  Audacity: A Digital Audio Editor

  ImportMIDI.cpp

  Dominic Mazzoni

**********************************************************************/

#include "../Audacity.h" // for USE_* macros
#include "ImportMIDI.h"

#include <wx/defs.h>
#include <wx/ffile.h>
#include <wx/frame.h>
#include <wx/intl.h>

#if defined(USE_MIDI)

//#include "allegro.h"
//#include "strparse.h"
//#include "mfmidi.h"

#include "../NoteTrack.h"
#include "../Project.h"
#include "../ProjectHistory.h"
#include "../ProjectWindow.h"
#include "../SelectUtilities.h"
#include "../widgets/AudacityMessageBox.h"
#include "../widgets/FileHistory.h"

// Given an existing project, try to import into it, return true on success
bool DoImportMIDI( AudacityProject &project, const FilePath &fileName )
{
   auto &tracks = TrackList::Get( project );
   auto newTrack = TrackFactory::Get( project ).NewNoteTrack();
   
   if (::ImportMIDI(fileName, newTrack.get())) {
      
      SelectUtilities::SelectNone( project );
      auto pTrack = tracks.Add( newTrack );
      pTrack->SetSelected(true);
      
      ProjectHistory::Get( project )
         .PushState(
            XO("Imported MIDI from '%s'").Format( fileName ),
            XO("Import MIDI")
         );
      
      ProjectWindow::Get( project ).ZoomAfterImport(pTrack);
      FileHistory::Global().AddFileToHistory(fileName);
      return true;
   }
   else
      return false;
}

bool ImportMIDI(const FilePath &fName, NoteTrack * dest)
{
   if (fName.length() <= 4){
      AudacityMessageBox(
         XO("Could not open file %s: Filename too short.").Format( fName ) );
      return false;
   }

   bool is_midi = false;
   if (fName.Right(4).CmpNoCase(wxT(".mid")) == 0 || fName.Right(5).CmpNoCase(wxT(".midi")) == 0)
      is_midi = true;
   else if(fName.Right(4).CmpNoCase(wxT(".gro")) != 0) {
      AudacityMessageBox(
         XO("Could not open file %s: Incorrect filetype.").Format( fName ) );
      return false;
   }

   wxFFile mf(fName, wxT("rb"));
   if (!mf.IsOpened()) {
      AudacityMessageBox(
         XO("Could not open file %s.").Format( fName ) );
      return false;
   }

   double offset = 0.0;
   auto new_seq = std::make_unique<Alg_seq>(fName.mb_str(), is_midi, &offset);

   //Should we also check if(seq->tracks() == 0) ?
   if(new_seq->get_read_error() == alg_error_open){
      AudacityMessageBox(
         XO("Could not open file %s.").Format( fName ) );
      mf.Close();
      return false;
   }

   dest->SetSequence(std::move(new_seq));
   dest->SetOffset(offset);
   wxString trackNameBase = fName.AfterLast(wxFILE_SEP_PATH).BeforeLast('.');
   dest->SetName(trackNameBase);
   mf.Close();

   dest->ZoomAllNotes();
   return true;
}

#endif
