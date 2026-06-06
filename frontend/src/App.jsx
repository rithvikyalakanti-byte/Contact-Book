import { useState, useEffect } from "react";

function App() {
  const [contacts, setContacts] = useState([]);
  const [name, setName] = useState("");
  const [phone, setPhone] = useState("");

  const API_URL = "http://localhost:8080/contacts";

  const fetchContacts = async () => {
    try {
      const response = await fetch(API_URL);
      const data = await response.json();
      setContacts(data);
    } catch (error) {
      console.error("Error fetching contacts:", error);
    }
  };

  useEffect(() => {
    fetchContacts();
  }, []);

  const handleSubmit = async (e) => {
    e.preventDefault();
    if (!name || !phone) return alert("Please fill out both fields");

    try {
      await fetch(API_URL, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ name, phone }),
      });
      setName("");
      setPhone("");
      fetchContacts();
    } catch (error) {
      console.error("Error adding contact:", error);
    }
  };

  return (
    <div style={{ maxWidth: "500px", margin: "50px auto", fontFamily: "Arial, sans-serif", padding: "20px", boxShadow: "0 0 10px rgba(0,0,0,0.1)", borderRadius: "8px", background: "#fff" }}>
      <h2 style={{ textAlign: "center", color: "#333" }}>📋 C & React Contact Book</h2>
      
      <form onSubmit={handleSubmit} style={{ display: "flex", flexDirection: "column", gap: "10px", marginBottom: "20px" }}>
        <input 
          type="text" 
          placeholder="Name" 
          value={name} 
          onChange={(e) => setName(e.target.value)}
          style={{ padding: "10px", fontSize: "16px", borderRadius: "4px", border: "1px solid #ccc" }}
        />
        <input 
          type="text" 
          placeholder="Phone Number" 
          value={phone} 
          onChange={(e) => setPhone(e.target.value)}
          style={{ padding: "10px", fontSize: "16px", borderRadius: "4px", border: "1px solid #ccc" }}
        />
        <button type="submit" style={{ padding: "10px", background: "#007BFF", color: "white", border: "none", borderRadius: "4px", cursor: "pointer", fontSize: "16px" }}>
          Add Contact
        </button>
      </form>

      <h3>Contact List</h3>
      <ul style={{ listStyle: "none", padding: 0 }}>
        {contacts.map((contact) => (
          <li key={contact.id} style={{ padding: "10px", borderBottom: "1px solid #ddd", display: "flex", justifyContent: "space-between", alignItems: "center" }}>
            <strong>{contact.name}</strong> 
            <span style={{ color: "#666" }}>{contact.phone}</span>
          </li>
        ))}
      </ul>
    </div>
  );
}

export default App;