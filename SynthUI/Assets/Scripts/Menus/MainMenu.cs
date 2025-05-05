using UnityEngine;
using UnityEngine.SceneManagement;

public class MainMenu : MonoBehaviour
{
    public void GoToInstrumentSelection() {
        SceneManager.LoadScene("InstrumentSelection");
    }

    public void GoToMainMenu() {
        SceneManager.LoadScene("MainMenu");
    }

    public void ExitApplication() {
        Debug.Log("Quitting Application..");
        Application.Quit();
    }
}
